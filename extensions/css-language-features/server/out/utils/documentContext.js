"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const strings_1 = require("../utils/strings");
const url = require("url");
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
            return url.resolve(base, ref);
        },
    };
}
exports.getDocumentContext = getDocumentContext;
//# sourceMappingURL=https://ticino.blob.core.windows.net/sourcemaps/0e4914e21e8fd9d3eb22a61780ef74692ab25bdc/extensions/css-language-features/server/out/utils/documentContext.js.map
