"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const path = require("path");
const vscode_1 = require("vscode");
function getCustomDataPathsInAllWorkspaces(workspaceFolders) {
    const dataPaths = [];
    if (!workspaceFolders) {
        return dataPaths;
    }
    workspaceFolders.forEach(wf => {
        const allCssConfig = vscode_1.workspace.getConfiguration(undefined, wf.uri);
        const wfCSSConfig = allCssConfig.inspect('css');
        if (wfCSSConfig &&
            wfCSSConfig.workspaceFolderValue &&
            wfCSSConfig.workspaceFolderValue.experimental &&
            wfCSSConfig.workspaceFolderValue.experimental.customData) {
            const customData = wfCSSConfig.workspaceFolderValue.experimental.customData;
            if (Array.isArray(customData)) {
                customData.forEach(t => {
                    if (typeof t === 'string') {
                        dataPaths.push(path.resolve(wf.uri.fsPath, t));
                    }
                });
            }
        }
    });
    return dataPaths;
}
exports.getCustomDataPathsInAllWorkspaces = getCustomDataPathsInAllWorkspaces;
function getCustomDataPathsFromAllExtensions() {
    const dataPaths = [];
    for (const extension of vscode_1.extensions.all) {
        const contributes = extension.packageJSON && extension.packageJSON.contributes;
        if (contributes && contributes.css && contributes.css.experimental.customData && Array.isArray(contributes.css.experimental.customData)) {
            const relativePaths = contributes.css.experimental.customData;
            relativePaths.forEach(rp => {
                dataPaths.push(path.resolve(extension.extensionPath, rp));
            });
        }
    }
    return dataPaths;
}
exports.getCustomDataPathsFromAllExtensions = getCustomDataPathsFromAllExtensions;
//# sourceMappingURL=customData.js.map