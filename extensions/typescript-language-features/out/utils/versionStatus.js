"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const vscode = require("vscode");
const nls = require("vscode-nls");
const arrays_1 = require("../utils/arrays");
const languageModeIds_1 = require("../utils/languageModeIds");
const tsconfig_1 = require("../utils/tsconfig");
const dispose_1 = require("./dispose");
const localize = nls.loadMessageBundle();
var ProjectInfoState;
(function (ProjectInfoState) {
    ProjectInfoState.None = Object.freeze({ type: 0 /* None */ });
    class Pending {
        constructor(resource) {
            this.resource = resource;
            this.type = 1 /* Pending */;
            this.cancellation = new vscode.CancellationTokenSource();
        }
    }
    ProjectInfoState.Pending = Pending;
    class Resolved {
        constructor(resource, configFile) {
            this.resource = resource;
            this.configFile = configFile;
            this.type = 2 /* Resolved */;
        }
    }
    ProjectInfoState.Resolved = Resolved;
})(ProjectInfoState || (ProjectInfoState = {}));
class ProjectStatusCommand {
    constructor(_client, _delegate) {
        this._client = _client;
        this._delegate = _delegate;
        this.id = '_typescript.projectStatus';
    }
    async execute() {
        const info = this._delegate();
        const result = await vscode.window.showQuickPick(arrays_1.coalesce([
            this.getProjectItem(info),
            this.getVersionItem(),
            this.getHelpItem(),
        ]), {
            placeHolder: localize('projectQuickPick.placeholder', "TypeScript Project Info"),
        });
        return result === null || result === void 0 ? void 0 : result.run();
    }
    getVersionItem() {
        return {
            label: localize('projectQuickPick.version.label', "Select TypeScript Version..."),
            description: this._client.apiVersion.displayName,
            run: () => {
                this._client.showVersionPicker();
            }
        };
    }
    getProjectItem(info) {
        const rootPath = info.type === 2 /* Resolved */ ? this._client.getWorkspaceRootForResource(info.resource) : undefined;
        if (!rootPath) {
            return undefined;
        }
        if (info.type === 2 /* Resolved */) {
            if (tsconfig_1.isImplicitProjectConfigFile(info.configFile)) {
                return {
                    label: localize('projectQuickPick.project.create', "Create tsconfig"),
                    detail: localize('projectQuickPick.project.create.description', "This file is currently not part of a tsconfig/jsconfig project"),
                    run: () => {
                        tsconfig_1.openOrCreateConfig(0 /* TypeScript */, rootPath, this._client.configuration);
                    }
                };
            }
        }
        return {
            label: localize('projectQuickPick.version.goProjectConfig', "Open tsconfig"),
            description: info.type === 2 /* Resolved */ ? vscode.workspace.asRelativePath(info.configFile) : undefined,
            run: () => {
                if (info.type === 2 /* Resolved */) {
                    tsconfig_1.openProjectConfigOrPromptToCreate(0 /* TypeScript */, this._client, rootPath, info.configFile);
                }
                else if (info.type === 1 /* Pending */) {
                    tsconfig_1.openProjectConfigForFile(0 /* TypeScript */, this._client, info.resource);
                }
            }
        };
    }
    getHelpItem() {
        return {
            label: localize('projectQuickPick.help', "TypeScript help"),
            run: () => {
                vscode.env.openExternal(vscode.Uri.parse('https://go.microsoft.com/fwlink/?linkid=839919')); // TODO:
            }
        };
    }
}
class VersionStatus extends dispose_1.Disposable {
    constructor(_client, commandManager) {
        super();
        this._client = _client;
        this._ready = false;
        this._state = ProjectInfoState.None;
        this._statusBarEntry = this._register(vscode.window.createStatusBarItem({
            id: 'status.typescript',
            name: localize('projectInfo.name', "TypeScript: Project Info"),
            alignment: vscode.StatusBarAlignment.Right,
            priority: 99 /* to the right of editor status (100) */
        }));
        const command = new ProjectStatusCommand(this._client, () => this._state);
        commandManager.register(command);
        this._statusBarEntry.command = command.id;
        vscode.window.onDidChangeActiveTextEditor(this.updateStatus, this, this._disposables);
        this._client.onReady(() => {
            this._ready = true;
            this.updateStatus();
        });
    }
    onDidChangeTypeScriptVersion(version) {
        this._statusBarEntry.text = version.displayName;
        this._statusBarEntry.tooltip = version.path;
        this.updateStatus();
    }
    async updateStatus() {
        if (!vscode.window.activeTextEditor) {
            this.hide();
            return;
        }
        const doc = vscode.window.activeTextEditor.document;
        if (languageModeIds_1.isTypeScriptDocument(doc)) {
            const file = this._client.normalizedPath(doc.uri);
            if (file) {
                this._statusBarEntry.show();
                if (!this._ready) {
                    return;
                }
                const pendingState = new ProjectInfoState.Pending(doc.uri);
                this.updateState(pendingState);
                const response = await this._client.execute('projectInfo', { file, needFileNameList: false }, pendingState.cancellation.token);
                if (response.type === 'response' && response.body) {
                    if (this._state === pendingState) {
                        this.updateState(new ProjectInfoState.Resolved(doc.uri, response.body.configFileName));
                        this._statusBarEntry.show();
                    }
                }
                return;
            }
        }
        if (!vscode.window.activeTextEditor.viewColumn) {
            // viewColumn is undefined for the debug/output panel, but we still want
            // to show the version info in the existing editor
            return;
        }
        this.hide();
    }
    hide() {
        this._statusBarEntry.hide();
        this.updateState(ProjectInfoState.None);
    }
    updateState(newState) {
        if (this._state === newState) {
            return;
        }
        if (this._state.type === 1 /* Pending */) {
            this._state.cancellation.cancel();
            this._state.cancellation.dispose();
        }
        this._state = newState;
    }
}
exports.default = VersionStatus;
//# sourceMappingURL=versionStatus.js.map