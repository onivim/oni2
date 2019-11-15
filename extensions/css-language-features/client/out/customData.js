"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
exports.__esModule = true;
var path = require("path");
var vscode_1 = require("vscode");
function getCustomDataPathsInAllWorkspaces(workspaceFolders) {
    var dataPaths = [];
    if (!workspaceFolders) {
        return dataPaths;
    }
    workspaceFolders.forEach(function (wf) {
        var allCssConfig = vscode_1.workspace.getConfiguration(undefined, wf.uri);
        var wfCSSConfig = allCssConfig.inspect('css');
        if (wfCSSConfig &&
            wfCSSConfig.workspaceFolderValue &&
            wfCSSConfig.workspaceFolderValue.experimental &&
            wfCSSConfig.workspaceFolderValue.experimental.customData) {
            var customData = wfCSSConfig.workspaceFolderValue.experimental.customData;
            if (Array.isArray(customData)) {
                customData.forEach(function (t) {
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
    var dataPaths = [];
    var _loop_1 = function (extension) {
        var contributes = extension.packageJSON && extension.packageJSON.contributes;
        if (contributes && contributes.css && contributes.css.experimental.customData && Array.isArray(contributes.css.experimental.customData)) {
            var relativePaths = contributes.css.experimental.customData;
            relativePaths.forEach(function (rp) {
                dataPaths.push(path.resolve(extension.extensionPath, rp));
            });
        }
    };
    for (var _i = 0, _a = vscode_1.extensions.all; _i < _a.length; _i++) {
        var extension = _a[_i];
        _loop_1(extension);
    }
    return dataPaths;
}
exports.getCustomDataPathsFromAllExtensions = getCustomDataPathsFromAllExtensions;
