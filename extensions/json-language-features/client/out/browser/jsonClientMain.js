"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = void 0;
const vscode_1 = require("vscode");
const jsonClient_1 = require("../jsonClient");
const browser_1 = require("vscode-languageclient/browser");
// this method is called when vs code is activated
function activate(context) {
    const serverMain = vscode_1.Uri.joinPath(context.extensionUri, 'server/dist/browser/jsonServerMain.js');
    try {
        const worker = new Worker(serverMain.toString());
        const newLanguageClient = (id, name, clientOptions) => {
            return new browser_1.LanguageClient(id, name, clientOptions, worker);
        };
        const http = {
            getContent(uri) {
                return fetch(uri, { mode: 'cors' })
                    .then(function (response) {
                    return response.text();
                });
            }
        };
        jsonClient_1.startClient(context, newLanguageClient, { http });
    }
    catch (e) {
        console.log(e);
    }
}
exports.activate = activate;
//# sourceMappingURL=jsonClientMain.js.map