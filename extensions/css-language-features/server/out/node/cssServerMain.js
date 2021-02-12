"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const node_1 = require("vscode-languageserver/node");
const runner_1 = require("../utils/runner");
const cssServer_1 = require("../cssServer");
const nodeFs_1 = require("./nodeFs");
// Create a connection for the server.
const connection = node_1.createConnection();
console.log = connection.console.log.bind(connection.console);
console.error = connection.console.error.bind(connection.console);
process.on('unhandledRejection', (e) => {
    connection.console.error(runner_1.formatError(`Unhandled exception`, e));
});
cssServer_1.startServer(connection, { file: nodeFs_1.getNodeFSRequestService() });
//# sourceMappingURL=cssServerMain.js.map