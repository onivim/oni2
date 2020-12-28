"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const vscode = require("vscode");
class TsConfigProvider {
    async getConfigsForWorkspace() {
        if (!vscode.workspace.workspaceFolders) {
            return [];
        }
        const configs = new Map();
        for (const config of await vscode.workspace.findFiles('**/tsconfig*.json', '**/{node_modules,.*}/**')) {
            const root = vscode.workspace.getWorkspaceFolder(config);
            if (root) {
                configs.set(config.fsPath, {
                    uri: config,
                    fsPath: config.fsPath,
                    posixPath: config.path,
                    workspaceFolder: root
                });
            }
        }
        return configs.values();
    }
}
exports.default = TsConfigProvider;
//# sourceMappingURL=tsconfigProvider.js.map