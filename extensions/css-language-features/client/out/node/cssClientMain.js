"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = void 0;
const nodeFs_1 = require("./nodeFs");
const vscode_1 = require("vscode");
const cssClient_1 = require("../cssClient");
const node_1 = require("vscode-languageclient/node");
const util_1 = require("util");
// this method is called when vs code is activated
function activate(context) {
    var _a, _b;
    const clientMain = ((_b = (_a = vscode_1.extensions.getExtension('vscode.css-language-features')) === null || _a === void 0 ? void 0 : _a.packageJSON) === null || _b === void 0 ? void 0 : _b.main) || '';
    const serverMain = `./server/${clientMain.indexOf('/dist/') !== -1 ? 'dist' : 'out'}/node/cssServerMain`;
    const serverModule = context.asAbsolutePath(serverMain);
    // The debug options for the server
    const debugOptions = { execArgv: ['--nolazy', '--inspect=' + (7000 + Math.round(Math.random() * 999))] };
    // If the extension is launch in debug mode the debug server options are use
    // Otherwise the run options are used
    const serverOptions = {
        run: { module: serverModule, transport: node_1.TransportKind.ipc },
        debug: { module: serverModule, transport: node_1.TransportKind.ipc, options: debugOptions }
    };
    const newLanguageClient = (id, name, clientOptions) => {
        return new node_1.LanguageClient(id, name, serverOptions, clientOptions);
    };
    cssClient_1.startClient(context, newLanguageClient, { fs: nodeFs_1.getNodeFSRequestService(), TextDecoder: util_1.TextDecoder });
}
exports.activate = activate;
//# sourceMappingURL=cssClientMain.js.map