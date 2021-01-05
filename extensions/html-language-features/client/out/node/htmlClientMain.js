"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = void 0;
const nodeFs_1 = require("./nodeFs");
const htmlClient_1 = require("../htmlClient");
const node_1 = require("vscode-languageclient/node");
const util_1 = require("util");
const fs = require("fs");
const vscode_extension_telemetry_1 = require("vscode-extension-telemetry");
let telemetry;
// this method is called when vs code is activated
function activate(context) {
    let clientPackageJSON = getPackageInfo(context);
    telemetry = new vscode_extension_telemetry_1.default(clientPackageJSON.name, clientPackageJSON.version, clientPackageJSON.aiKey);
    const serverMain = `./server/${clientPackageJSON.main.indexOf('/dist/') !== -1 ? 'dist' : 'out'}/node/htmlServerMain`;
    const serverModule = context.asAbsolutePath(serverMain);
    // The debug options for the server
    const debugOptions = { execArgv: ['--nolazy', '--inspect=' + (8000 + Math.round(Math.random() * 999))] };
    // If the extension is launch in debug mode the debug server options are use
    // Otherwise the run options are used
    const serverOptions = {
        run: { module: serverModule, transport: node_1.TransportKind.ipc },
        debug: { module: serverModule, transport: node_1.TransportKind.ipc, options: debugOptions }
    };
    const newLanguageClient = (id, name, clientOptions) => {
        return new node_1.LanguageClient(id, name, serverOptions, clientOptions);
    };
    htmlClient_1.startClient(context, newLanguageClient, { fs: nodeFs_1.getNodeFSRequestService(), TextDecoder: util_1.TextDecoder, telemetry });
}
exports.activate = activate;
function getPackageInfo(context) {
    const location = context.asAbsolutePath('./package.json');
    try {
        return JSON.parse(fs.readFileSync(location).toString());
    }
    catch (e) {
        console.log(`Problems reading ${location}: ${e}`);
        return { name: '', version: '', aiKey: '', main: '' };
    }
}
//# sourceMappingURL=htmlClientMain.js.map