"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.loadLibrary = void 0;
const path_1 = require("path");
const fs_1 = require("fs");
const contents = {};
const serverFolder = path_1.basename(__dirname) === 'dist' ? path_1.dirname(__dirname) : path_1.dirname(path_1.dirname(__dirname));
const TYPESCRIPT_LIB_SOURCE = path_1.join(serverFolder, '../../node_modules/typescript/lib');
const JQUERY_PATH = path_1.join(serverFolder, 'lib/jquery.d.ts');
function loadLibrary(name) {
    let content = contents[name];
    if (typeof content !== 'string') {
        let libPath;
        if (name === 'jquery') {
            libPath = JQUERY_PATH;
        }
        else {
            libPath = path_1.join(TYPESCRIPT_LIB_SOURCE, name); // from source
        }
        try {
            content = fs_1.readFileSync(libPath).toString();
        }
        catch (e) {
            console.log(`Unable to load library ${name} at ${libPath}: ${e.message}`);
            content = '';
        }
        contents[name] = content;
    }
    return content;
}
exports.loadLibrary = loadLibrary;
//# sourceMappingURL=javascriptLibs.js.map