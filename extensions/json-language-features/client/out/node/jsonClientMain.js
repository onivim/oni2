"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.deactivate = exports.activate = void 0;
const jsonClient_1 = require("../jsonClient");
const node_1 = require("vscode-languageclient/node");
const fs = require("fs");
const request_light_1 = require("request-light");
const vscode_extension_telemetry_1 = require("vscode-extension-telemetry");
let telemetry;
// this method is called when vs code is activated
function activate(context) {
    const clientPackageJSON = getPackageInfo(context);
    telemetry = new vscode_extension_telemetry_1.default(clientPackageJSON.name, clientPackageJSON.version, clientPackageJSON.aiKey);
    const serverMain = `./server/${clientPackageJSON.main.indexOf('/dist/') !== -1 ? 'dist' : 'out'}/node/jsonServerMain`;
    const serverModule = context.asAbsolutePath(serverMain);
    // The debug options for the server
    const debugOptions = { execArgv: ['--nolazy', '--inspect=' + (6000 + Math.round(Math.random() * 999))] };
    // If the extension is launch in debug mode the debug server options are use
    // Otherwise the run options are used
    const serverOptions = {
        run: { module: serverModule, transport: node_1.TransportKind.ipc },
        debug: { module: serverModule, transport: node_1.TransportKind.ipc, options: debugOptions }
    };
    const newLanguageClient = (id, name, clientOptions) => {
        return new node_1.LanguageClient(id, name, serverOptions, clientOptions);
    };
    jsonClient_1.startClient(context, newLanguageClient, { http: getHTTPRequestService(), telemetry });
}
exports.activate = activate;
function deactivate() {
    return telemetry ? telemetry.dispose() : Promise.resolve(null);
}
exports.deactivate = deactivate;
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
function getHTTPRequestService() {
    return {
        getContent(uri, _encoding) {
            const headers = { 'Accept-Encoding': 'gzip, deflate' };
            return request_light_1.xhr({ url: uri, followRedirects: 5, headers }).then(response => {
                return response.responseText;
            }, (error) => {
                return Promise.reject(error.responseText || request_light_1.getErrorStatusDescription(error.status) || error.toString());
            });
        }
    };
}
//# sourceMappingURL=jsonClientMain.js.map