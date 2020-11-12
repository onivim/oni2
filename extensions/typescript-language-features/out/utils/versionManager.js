"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.TypeScriptVersionManager = void 0;
const vscode = require("vscode");
const nls = require("vscode-nls");
const dispose_1 = require("./dispose");
const localize = nls.loadMessageBundle();
const useWorkspaceTsdkStorageKey = 'typescript.useWorkspaceTsdk';
class TypeScriptVersionManager extends dispose_1.Disposable {
    constructor(versionProvider, workspaceState) {
        super();
        this.versionProvider = versionProvider;
        this.workspaceState = workspaceState;
        this._onDidPickNewVersion = this._register(new vscode.EventEmitter());
        this.onDidPickNewVersion = this._onDidPickNewVersion.event;
        this._currentVersion = this.versionProvider.defaultVersion;
        if (this.useWorkspaceTsdkSetting) {
            const localVersion = this.versionProvider.localVersion;
            if (localVersion) {
                this._currentVersion = localVersion;
            }
        }
    }
    get currentVersion() {
        return this._currentVersion;
    }
    reset() {
        this._currentVersion = this.versionProvider.bundledVersion;
    }
    async promptUserForVersion() {
        const selected = await vscode.window.showQuickPick([
            this.getBundledPickItem(),
            ...this.getLocalPickItems(),
            LearnMorePickItem,
        ], {
            placeHolder: localize('selectTsVersion', "Select the TypeScript version used for JavaScript and TypeScript language features"),
        });
        return selected === null || selected === void 0 ? void 0 : selected.run();
    }
    getBundledPickItem() {
        const bundledVersion = this.versionProvider.defaultVersion;
        return {
            label: (!this.useWorkspaceTsdkSetting
                ? '• '
                : '') + localize('useVSCodeVersionOption', "Use VS Code's Version"),
            description: bundledVersion.displayName,
            detail: bundledVersion.pathLabel,
            run: async () => {
                await this.workspaceState.update(useWorkspaceTsdkStorageKey, false);
                this.updateForPickedVersion(bundledVersion);
            },
        };
    }
    getLocalPickItems() {
        return this.versionProvider.localVersions.map(version => {
            return {
                label: (this.useWorkspaceTsdkSetting && this.currentVersion.eq(version)
                    ? '• '
                    : '') + localize('useWorkspaceVersionOption', "Use Workspace Version"),
                description: version.displayName,
                detail: version.pathLabel,
                run: async () => {
                    await this.workspaceState.update(useWorkspaceTsdkStorageKey, true);
                    const tsConfig = vscode.workspace.getConfiguration('typescript');
                    await tsConfig.update('tsdk', version.pathLabel, false);
                    this.updateForPickedVersion(version);
                },
            };
        });
    }
    updateForPickedVersion(pickedVersion) {
        const oldVersion = this.currentVersion;
        this._currentVersion = pickedVersion;
        if (!oldVersion.eq(pickedVersion)) {
            this._onDidPickNewVersion.fire();
        }
    }
    get useWorkspaceTsdkSetting() {
        return this.workspaceState.get(useWorkspaceTsdkStorageKey, false);
    }
}
exports.TypeScriptVersionManager = TypeScriptVersionManager;
const LearnMorePickItem = {
    label: localize('learnMore', 'Learn more about managing TypeScript versions'),
    description: '',
    run: () => {
        vscode.env.openExternal(vscode.Uri.parse('https://go.microsoft.com/fwlink/?linkid=839919'));
    }
};
//# sourceMappingURL=versionManager.js.map