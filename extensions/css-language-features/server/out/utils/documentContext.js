"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.getDocumentContext = void 0;
const strings_1 = require("../utils/strings");
const url = require("url");
const vscode_uri_1 = require("vscode-uri");
const path_1 = require("path");
const fs_1 = require("fs");
function getModuleNameFromPath(path) {
    // If a scoped module (starts with @) then get up until second instance of '/', otherwise get until first isntance of '/'
    if (path[0] === '@') {
        return path.substring(0, path.indexOf('/', path.indexOf('/') + 1));
    }
    return path.substring(0, path.indexOf('/'));
}
function resolvePathToModule(_moduleName, _relativeTo) {
    // resolve the module relative to the document. We can't use `require` here as the code is webpacked.
    const documentFolder = path_1.dirname(vscode_uri_1.URI.parse(_relativeTo).fsPath);
    const packPath = path_1.join(documentFolder, 'node_modules', _moduleName, 'package.json');
    if (fs_1.existsSync(packPath)) {
        return vscode_uri_1.URI.file(packPath).toString();
    }
    return undefined;
}
function getDocumentContext(documentUri, workspaceFolders) {
    function getRootFolder() {
        for (let folder of workspaceFolders) {
            let folderURI = folder.uri;
            if (!strings_1.endsWith(folderURI, '/')) {
                folderURI = folderURI + '/';
            }
            if (strings_1.startsWith(documentUri, folderURI)) {
                return folderURI;
            }
        }
        return undefined;
    }
    return {
        resolveReference: (ref, base = documentUri) => {
            if (ref[0] === '/') { // resolve absolute path against the current workspace folder
                if (strings_1.startsWith(base, 'file://')) {
                    let folderUri = getRootFolder();
                    if (folderUri) {
                        return folderUri + ref.substr(1);
                    }
                }
            }
            // Following [css-loader](https://github.com/webpack-contrib/css-loader#url)
            // and [sass-loader's](https://github.com/webpack-contrib/sass-loader#imports)
            // convention, if an import path starts with ~ then use node module resolution
            // *unless* it starts with "~/" as this refers to the user's home directory.
            if (ref[0] === '~' && ref[1] !== '/') {
                ref = ref.substring(1);
                if (strings_1.startsWith(base, 'file://')) {
                    const moduleName = getModuleNameFromPath(ref);
                    const modulePath = resolvePathToModule(moduleName, base);
                    if (modulePath) {
                        const pathWithinModule = ref.substring(moduleName.length + 1);
                        return url.resolve(modulePath, pathWithinModule);
                    }
                }
            }
            return url.resolve(base, ref);
        },
    };
}
exports.getDocumentContext = getDocumentContext;
//# sourceMappingURL=documentContext.js.map