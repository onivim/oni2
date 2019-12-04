"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const fs = require("fs");
const path = require("path");
const vscode = require("vscode");
const nls = require("vscode-nls");
const api_1 = require("../utils/api");
const cancellation_1 = require("../utils/cancellation");
const dependentRegistration_1 = require("../utils/dependentRegistration");
const dispose_1 = require("../utils/dispose");
const fileSchemes = require("../utils/fileSchemes");
const languageModeIds_1 = require("../utils/languageModeIds");
const regexp_1 = require("../utils/regexp");
const typeConverters = require("../utils/typeConverters");
const localize = nls.loadMessageBundle();
const updateImportsOnFileMoveName = 'updateImportsOnFileMove.enabled';
var UpdateImportsOnFileMoveSetting;
(function (UpdateImportsOnFileMoveSetting) {
    UpdateImportsOnFileMoveSetting["Prompt"] = "prompt";
    UpdateImportsOnFileMoveSetting["Always"] = "always";
    UpdateImportsOnFileMoveSetting["Never"] = "never";
})(UpdateImportsOnFileMoveSetting || (UpdateImportsOnFileMoveSetting = {}));
class UpdateImportsOnFileRenameHandler extends dispose_1.Disposable {
    constructor(client, fileConfigurationManager, _handles) {
        super();
        this.client = client;
        this.fileConfigurationManager = fileConfigurationManager;
        this._handles = _handles;
        this._register(vscode.workspace.onDidRenameFile(e => {
            vscode.window.withProgress({
                location: vscode.ProgressLocation.Window,
                title: localize('renameProgress.title', "Checking for update of JS/TS imports")
            }, () => {
                return this.doRename(e.oldUri, e.newUri);
            });
        }));
    }
    async doRename(oldResource, newResource) {
        const targetResource = await this.getTargetResource(newResource);
        if (!targetResource) {
            return;
        }
        const targetFile = this.client.toPath(targetResource);
        if (!targetFile) {
            return;
        }
        const newFile = this.client.toPath(newResource);
        if (!newFile) {
            return;
        }
        const oldFile = this.client.toPath(oldResource);
        if (!oldFile) {
            return;
        }
        const document = await vscode.workspace.openTextDocument(targetResource);
        const config = this.getConfiguration(document);
        const setting = config.get(updateImportsOnFileMoveName);
        if (setting === UpdateImportsOnFileMoveSetting.Never) {
            return;
        }
        // Make sure TS knows about file
        this.client.bufferSyncSupport.closeResource(oldResource);
        this.client.bufferSyncSupport.openTextDocument(document);
        if (this.client.apiVersion.lt(api_1.default.v300) && !fs.lstatSync(newResource.fsPath).isDirectory()) {
            // Workaround for https://github.com/Microsoft/vscode/issues/52967
            // Never attempt to update import paths if the file does not contain something the looks like an export
            try {
                const response = await this.client.execute('navtree', { file: newFile }, cancellation_1.nulToken);
                if (response.type !== 'response' || !response.body) {
                    return;
                }
                const hasExport = (node) => {
                    return !!node.kindModifiers.match(/\bexports?\b/g) || !!(node.childItems && node.childItems.some(hasExport));
                };
                if (!hasExport(response.body)) {
                    return;
                }
            }
            catch (_a) {
                // noop
            }
        }
        const edits = await this.getEditsForFileRename(targetFile, document, oldFile, newFile);
        if (!edits || !edits.size) {
            return;
        }
        if (await this.confirmActionWithUser(newResource, document)) {
            await vscode.workspace.applyEdit(edits);
        }
    }
    async confirmActionWithUser(newResource, newDocument) {
        const config = this.getConfiguration(newDocument);
        const setting = config.get(updateImportsOnFileMoveName);
        switch (setting) {
            case UpdateImportsOnFileMoveSetting.Always:
                return true;
            case UpdateImportsOnFileMoveSetting.Never:
                return false;
            case UpdateImportsOnFileMoveSetting.Prompt:
            default:
                return this.promptUser(newResource, newDocument);
        }
    }
    getConfiguration(newDocument) {
        return vscode.workspace.getConfiguration(languageModeIds_1.isTypeScriptDocument(newDocument) ? 'typescript' : 'javascript', newDocument.uri);
    }
    async promptUser(newResource, newDocument) {
        let Choice;
        (function (Choice) {
            Choice[Choice["None"] = 0] = "None";
            Choice[Choice["Accept"] = 1] = "Accept";
            Choice[Choice["Reject"] = 2] = "Reject";
            Choice[Choice["Always"] = 3] = "Always";
            Choice[Choice["Never"] = 4] = "Never";
        })(Choice || (Choice = {}));
        const response = await vscode.window.showInformationMessage(localize('prompt', "Update imports for moved file: '{0}'?", path.basename(newResource.fsPath)), {
            modal: true,
        }, {
            title: localize('reject.title', "No"),
            choice: Choice.Reject,
            isCloseAffordance: true,
        }, {
            title: localize('accept.title', "Yes"),
            choice: Choice.Accept,
        }, {
            title: localize('always.title', "Always automatically update imports"),
            choice: Choice.Always,
        }, {
            title: localize('never.title', "Never automatically update imports"),
            choice: Choice.Never,
        });
        if (!response) {
            return false;
        }
        switch (response.choice) {
            case Choice.Accept:
                {
                    return true;
                }
            case Choice.Reject:
                {
                    return false;
                }
            case Choice.Always:
                {
                    const config = this.getConfiguration(newDocument);
                    config.update(updateImportsOnFileMoveName, UpdateImportsOnFileMoveSetting.Always, vscode.ConfigurationTarget.Global);
                    return true;
                }
            case Choice.Never:
                {
                    const config = this.getConfiguration(newDocument);
                    config.update(updateImportsOnFileMoveName, UpdateImportsOnFileMoveSetting.Never, vscode.ConfigurationTarget.Global);
                    return false;
                }
        }
        return false;
    }
    async getTargetResource(resource) {
        if (resource.scheme !== fileSchemes.file) {
            return undefined;
        }
        const isDirectory = fs.lstatSync(resource.fsPath).isDirectory();
        if (isDirectory && this.client.apiVersion.gte(api_1.default.v300)) {
            return resource;
        }
        if (isDirectory && this.client.apiVersion.gte(api_1.default.v292)) {
            const files = await vscode.workspace.findFiles({
                base: resource.fsPath,
                pattern: '**/*.{ts,tsx,js,jsx}',
            }, '**/node_modules/**', 1);
            return files[0];
        }
        return (await this._handles(resource)) ? resource : undefined;
    }
    async getEditsForFileRename(targetResource, document, oldFile, newFile) {
        const isDirectoryRename = fs.lstatSync(newFile).isDirectory();
        const response = await this.client.interruptGetErr(() => {
            this.fileConfigurationManager.setGlobalConfigurationFromDocument(document, cancellation_1.nulToken);
            const args = {
                file: targetResource,
                oldFilePath: oldFile,
                newFilePath: newFile,
            };
            return this.client.execute('getEditsForFileRename', args, cancellation_1.nulToken);
        });
        if (response.type !== 'response' || !response.body) {
            return;
        }
        const edits = [];
        for (const edit of response.body) {
            // Workaround for https://github.com/Microsoft/vscode/issues/52675
            if (this.client.apiVersion.lt(api_1.default.v300)) {
                if (edit.fileName.match(/[\/\\]node_modules[\/\\]/gi)) {
                    continue;
                }
                for (const change of edit.textChanges) {
                    if (change.newText.match(/\/node_modules\//gi)) {
                        continue;
                    }
                }
            }
            edits.push(await this.fixEdit(edit, isDirectoryRename, oldFile, newFile));
        }
        return typeConverters.WorkspaceEdit.fromFileCodeEdits(this.client, edits);
    }
    async fixEdit(edit, isDirectoryRename, oldFile, newFile) {
        if (!isDirectoryRename || this.client.apiVersion.gte(api_1.default.v300)) {
            return edit;
        }
        const document = await vscode.workspace.openTextDocument(edit.fileName);
        const oldFileRe = new RegExp('/' + regexp_1.escapeRegExp(path.basename(oldFile)) + '/');
        // Workaround for https://github.com/Microsoft/TypeScript/issues/24968
        const textChanges = edit.textChanges.map((change) => {
            const existingText = document.getText(typeConverters.Range.fromTextSpan(change));
            const existingMatch = existingText.match(oldFileRe);
            if (!existingMatch) {
                return change;
            }
            const match = new RegExp('/' + regexp_1.escapeRegExp(path.basename(newFile)) + '/(.+)$', 'g').exec(change.newText);
            if (!match) {
                return change;
            }
            return {
                newText: change.newText.slice(0, -match[1].length),
                start: change.start,
                end: {
                    line: change.end.line,
                    offset: change.end.offset - match[1].length,
                },
            };
        });
        return {
            fileName: edit.fileName,
            textChanges,
        };
    }
}
function register(client, fileConfigurationManager, handles) {
    return new dependentRegistration_1.VersionDependentRegistration(client, api_1.default.v290, () => new UpdateImportsOnFileRenameHandler(client, fileConfigurationManager, handles));
}
exports.register = register;
//# sourceMappingURL=updatePathsOnRename.js.map