"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = void 0;
const vscode_1 = require("vscode");
const htmlClient_1 = require("../htmlClient");
const browser_1 = require("vscode-languageclient/browser");
// this method is called when vs code is activated
function activate(context) {
    const serverMain = vscode_1.Uri.joinPath(context.extensionUri, 'server/dist/browser/htmlServerMain.js');
    try {
        const worker = new Worker(serverMain.toString());
        const newLanguageClient = (id, name, clientOptions) => {
            return new browser_1.LanguageClient(id, name, clientOptions, worker);
        };
        htmlClient_1.startClient(context, newLanguageClient, { TextDecoder });
    }
    catch (e) {
        console.log(e);
    }
}
exports.activate = activate;
//# sourceMappingURL=htmlClientMain.js.map