"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.LineDecoder = void 0;
const cp = require("child_process");
const string_decoder_1 = require("string_decoder");
const which = require("which");
const path = require("path");
const vscode = require("vscode");
const async_1 = require("./utils/async");
const nls = require("vscode-nls");
let localize = nls.loadMessageBundle();
class LineDecoder {
    constructor(encoding = 'utf8') {
        this.stringDecoder = new string_decoder_1.StringDecoder(encoding);
        this.remaining = null;
    }
    write(buffer) {
        let result = [];
        let value = this.remaining
            ? this.remaining + this.stringDecoder.write(buffer)
            : this.stringDecoder.write(buffer);
        if (value.length < 1) {
            return result;
        }
        let start = 0;
        let ch;
        while (start < value.length && ((ch = value.charCodeAt(start)) === 13 || ch === 10)) {
            start++;
        }
        let idx = start;
        while (idx < value.length) {
            ch = value.charCodeAt(idx);
            if (ch === 13 || ch === 10) {
                result.push(value.substring(start, idx));
                idx++;
                while (idx < value.length && ((ch = value.charCodeAt(idx)) === 13 || ch === 10)) {
                    idx++;
                }
                start = idx;
            }
            else {
                idx++;
            }
        }
        this.remaining = start < value.length ? value.substr(start) : null;
        return result;
    }
    end() {
        return this.remaining;
    }
}
exports.LineDecoder = LineDecoder;
var RunTrigger;
(function (RunTrigger) {
    RunTrigger[RunTrigger["onSave"] = 0] = "onSave";
    RunTrigger[RunTrigger["onType"] = 1] = "onType";
})(RunTrigger || (RunTrigger = {}));
(function (RunTrigger) {
    RunTrigger.strings = {
        onSave: 'onSave',
        onType: 'onType'
    };
    RunTrigger.from = function (value) {
        if (value === 'onType') {
            return RunTrigger.onType;
        }
        else {
            return RunTrigger.onSave;
        }
    };
})(RunTrigger || (RunTrigger = {}));
class PHPValidationProvider {
    constructor(workspaceStore) {
        this.workspaceStore = workspaceStore;
        this.documentListener = null;
        this.validationEnabled = true;
        this.pauseValidation = false;
        this.loadConfigP = this.loadConfiguration();
    }
    activate(subscriptions) {
        this.diagnosticCollection = vscode.languages.createDiagnosticCollection();
        subscriptions.push(this);
        subscriptions.push(vscode.workspace.onDidChangeConfiguration(() => this.loadConfigP = this.loadConfiguration()));
        vscode.workspace.onDidOpenTextDocument(this.triggerValidate, this, subscriptions);
        vscode.workspace.onDidCloseTextDocument((textDocument) => {
            this.diagnosticCollection.delete(textDocument.uri);
            delete this.delayers[textDocument.uri.toString()];
        }, null, subscriptions);
        subscriptions.push(vscode.commands.registerCommand('php.untrustValidationExecutable', this.untrustValidationExecutable, this));
    }
    dispose() {
        if (this.diagnosticCollection) {
            this.diagnosticCollection.clear();
            this.diagnosticCollection.dispose();
        }
        if (this.documentListener) {
            this.documentListener.dispose();
            this.documentListener = null;
        }
    }
    async loadConfiguration() {
        var _a;
        const section = vscode.workspace.getConfiguration();
        const oldExecutable = (_a = this.config) === null || _a === void 0 ? void 0 : _a.executable;
        this.validationEnabled = section.get("php.validate.enable" /* Enable */, true);
        this.config = await getConfig();
        if (this.config.executableIsUserDefined !== true && this.workspaceStore.get("php.validate.checkedExecutablePath" /* CheckedExecutablePath */, undefined) !== undefined) {
            vscode.commands.executeCommand('setContext', 'php.untrustValidationExecutableContext', true);
        }
        const trustEnabled = vscode.workspace.getConfiguration().get('security.workspace.trust.enabled');
        if (trustEnabled) {
            vscode.workspace.requestWorkspaceTrust();
        }
        this.delayers = Object.create(null);
        if (this.pauseValidation) {
            this.pauseValidation = oldExecutable === this.config.executable;
        }
        if (this.documentListener) {
            this.documentListener.dispose();
            this.documentListener = null;
        }
        this.diagnosticCollection.clear();
        if (this.validationEnabled) {
            if (this.config.trigger === RunTrigger.onType) {
                this.documentListener = vscode.workspace.onDidChangeTextDocument((e) => {
                    this.triggerValidate(e.document);
                });
            }
            else {
                this.documentListener = vscode.workspace.onDidSaveTextDocument(this.triggerValidate, this);
            }
            // Configuration has changed. Reevaluate all documents.
            vscode.workspace.textDocuments.forEach(this.triggerValidate, this);
        }
    }
    untrustValidationExecutable() {
        this.workspaceStore.update("php.validate.checkedExecutablePath" /* CheckedExecutablePath */, undefined);
        vscode.commands.executeCommand('setContext', 'php.untrustValidationExecutableContext', false);
    }
    async triggerValidate(textDocument) {
        await this.loadConfigP;
        if (textDocument.languageId !== 'php' || this.pauseValidation || !this.validationEnabled) {
            return;
        }
        let trigger = () => {
            var _a;
            let key = textDocument.uri.toString();
            let delayer = this.delayers[key];
            if (!delayer) {
                delayer = new async_1.ThrottledDelayer(((_a = this.config) === null || _a === void 0 ? void 0 : _a.trigger) === RunTrigger.onType ? 250 : 0);
                this.delayers[key] = delayer;
            }
            delayer.trigger(() => this.doValidate(textDocument));
        };
        const trustEnabled = vscode.workspace.getConfiguration().get('security.workspace.trust.enabled');
        if (trustEnabled) {
            if (vscode.workspace.isTrusted) {
                trigger();
            }
        }
        else if (this.config.executableIsUserDefined !== undefined && !this.config.executableIsUserDefined) {
            const checkedExecutablePath = this.workspaceStore.get("php.validate.checkedExecutablePath" /* CheckedExecutablePath */, undefined);
            if (!checkedExecutablePath || checkedExecutablePath !== this.config.executable) {
                if (await this.showCustomTrustDialog()) {
                    this.workspaceStore.update("php.validate.checkedExecutablePath" /* CheckedExecutablePath */, this.config.executable);
                    vscode.commands.executeCommand('setContext', 'php.untrustValidationExecutableContext', true);
                }
                else {
                    this.pauseValidation = true;
                    return;
                }
            }
            trigger();
        }
    }
    async showCustomTrustDialog() {
        const selected = await vscode.window.showInformationMessage(localize('php.useExecutablePath', 'Do you allow {0} (defined as a workspace setting) to be executed to lint PHP files?', this.config.executable), {
            title: localize('php.yes', 'Allow'),
            id: 'yes'
        }, {
            title: localize('php.no', 'Disallow'),
            isCloseAffordance: true,
            id: 'no'
        });
        if (selected && selected.id === 'yes') {
            return true;
        }
        return false;
    }
    doValidate(textDocument) {
        return new Promise(async (resolve) => {
            const executable = this.config.executable;
            if (!executable) {
                this.showErrorMessage(localize('noPhp', 'Cannot validate since a PHP installation could not be found. Use the setting \'php.validate.executablePath\' to configure the PHP executable.'));
                this.pauseValidation = true;
                resolve();
                return;
            }
            if (!path.isAbsolute(executable)) {
                // executable should either be resolved to an absolute path or undefined.
                // This is just to be sure.
                return;
            }
            let decoder = new LineDecoder();
            let diagnostics = [];
            let processLine = (line) => {
                let matches = line.match(PHPValidationProvider.MatchExpression);
                if (matches) {
                    let message = matches[1];
                    let line = parseInt(matches[3]) - 1;
                    let diagnostic = new vscode.Diagnostic(new vscode.Range(line, 0, line, Number.MAX_VALUE), message);
                    diagnostics.push(diagnostic);
                }
            };
            let options = (vscode.workspace.workspaceFolders && vscode.workspace.workspaceFolders[0]) ? { cwd: vscode.workspace.workspaceFolders[0].uri.fsPath } : undefined;
            let args;
            if (this.config.trigger === RunTrigger.onSave) {
                args = PHPValidationProvider.FileArgs.slice(0);
                args.push(textDocument.fileName);
            }
            else {
                args = PHPValidationProvider.BufferArgs;
            }
            try {
                let childProcess = cp.spawn(executable, args, options);
                childProcess.on('error', (error) => {
                    if (this.pauseValidation) {
                        resolve();
                        return;
                    }
                    this.showError(error, executable);
                    this.pauseValidation = true;
                    resolve();
                });
                if (childProcess.pid) {
                    if (this.config.trigger === RunTrigger.onType) {
                        childProcess.stdin.write(textDocument.getText());
                        childProcess.stdin.end();
                    }
                    childProcess.stdout.on('data', (data) => {
                        decoder.write(data).forEach(processLine);
                    });
                    childProcess.stdout.on('end', () => {
                        let line = decoder.end();
                        if (line) {
                            processLine(line);
                        }
                        this.diagnosticCollection.set(textDocument.uri, diagnostics);
                        resolve();
                    });
                }
                else {
                    resolve();
                }
            }
            catch (error) {
                this.showError(error, executable);
            }
        });
    }
    async showError(error, executable) {
        let message = null;
        if (error.code === 'ENOENT') {
            if (this.config.executable) {
                message = localize('wrongExecutable', 'Cannot validate since {0} is not a valid php executable. Use the setting \'php.validate.executablePath\' to configure the PHP executable.', executable);
            }
            else {
                message = localize('noExecutable', 'Cannot validate since no PHP executable is set. Use the setting \'php.validate.executablePath\' to configure the PHP executable.');
            }
        }
        else {
            message = error.message ? error.message : localize('unknownReason', 'Failed to run php using path: {0}. Reason is unknown.', executable);
        }
        if (!message) {
            return;
        }
        return this.showErrorMessage(message);
    }
    async showErrorMessage(message) {
        const openSettings = localize('goToSetting', 'Open Settings');
        if (await vscode.window.showInformationMessage(message, openSettings) === openSettings) {
            vscode.commands.executeCommand('workbench.action.openSettings', "php.validate.executablePath" /* ExecutablePath */);
        }
    }
}
exports.default = PHPValidationProvider;
PHPValidationProvider.MatchExpression = /(?:(?:Parse|Fatal) error): (.*)(?: in )(.*?)(?: on line )(\d+)/;
PHPValidationProvider.BufferArgs = ['-l', '-n', '-d', 'display_errors=On', '-d', 'log_errors=Off'];
PHPValidationProvider.FileArgs = ['-l', '-n', '-d', 'display_errors=On', '-d', 'log_errors=Off', '-f'];
async function getConfig() {
    const section = vscode.workspace.getConfiguration();
    let executable;
    let executableIsUserDefined;
    const inspect = section.inspect("php.validate.executablePath" /* ExecutablePath */);
    if (inspect && inspect.workspaceValue) {
        executable = inspect.workspaceValue;
        executableIsUserDefined = false;
    }
    else if (inspect && inspect.globalValue) {
        executable = inspect.globalValue;
        executableIsUserDefined = true;
    }
    else {
        executable = undefined;
        executableIsUserDefined = undefined;
    }
    if (executable && !path.isAbsolute(executable)) {
        const first = vscode.workspace.workspaceFolders && vscode.workspace.workspaceFolders[0];
        if (first) {
            executable = vscode.Uri.joinPath(first.uri, executable).fsPath;
        }
        else {
            executable = undefined;
        }
    }
    else if (!executable) {
        executable = await getPhpPath();
    }
    const trigger = RunTrigger.from(section.get("php.validate.run" /* Run */, RunTrigger.strings.onSave));
    return {
        executable,
        executableIsUserDefined,
        trigger
    };
}
async function getPhpPath() {
    try {
        return await which('php');
    }
    catch (e) {
        return undefined;
    }
}
//# sourceMappingURL=validationProvider.js.map