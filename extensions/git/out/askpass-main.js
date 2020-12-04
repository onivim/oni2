"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const fs = require("fs");
const nls = require("vscode-nls");
const ipcClient_1 = require("./ipc/ipcClient");
const localize = nls.loadMessageBundle();
function fatal(err) {
    console.error(localize('missOrInvalid', "Missing or invalid credentials."));
    console.error(err);
    process.exit(1);
}
function main(argv) {
    if (argv.length !== 5) {
        return fatal('Wrong number of arguments');
    }
    if (!process.env['VSCODE_GIT_ASKPASS_PIPE']) {
        return fatal('Missing pipe');
    }
    if (process.env['VSCODE_GIT_COMMAND'] === 'fetch' && !!process.env['VSCODE_GIT_FETCH_SILENT']) {
        return fatal('Skip silent fetch commands');
    }
    const output = process.env['VSCODE_GIT_ASKPASS_PIPE'];
    const request = argv[2];
    const host = argv[4].substring(1, argv[4].length - 2);
    const ipcClient = new ipcClient_1.IPCClient('askpass');
    ipcClient.call({ request, host }).then(res => {
        fs.writeFileSync(output, res + '\n');
        setTimeout(() => process.exit(0), 0);
    }).catch(err => fatal(err));
}
main(process.argv);
//# sourceMappingURL=askpass-main.js.map