"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const vscode_1 = require("vscode");
const util_1 = require("./util");
const path = require("path");
const http = require("http");
const os = require("os");
const fs = require("fs");
const crypto = require("crypto");
const randomBytes = util_1.denodeify(crypto.randomBytes);
function getIPCHandlePath(nonce) {
    if (process.platform === 'win32') {
        return `\\\\.\\pipe\\vscode-git-askpass-${nonce}-sock`;
    }
    if (process.env['XDG_RUNTIME_DIR']) {
        return path.join(process.env['XDG_RUNTIME_DIR'], `vscode-git-askpass-${nonce}.sock`);
    }
    return path.join(os.tmpdir(), `vscode-git-askpass-${nonce}.sock`);
}
class Askpass {
    constructor() {
        this.enabled = true;
        this.server = http.createServer((req, res) => this.onRequest(req, res));
        this.ipcHandlePathPromise = this.setup().catch(err => {
            console.error(err);
            return '';
        });
    }
    async setup() {
        const buffer = await randomBytes(20);
        const nonce = buffer.toString('hex');
        const ipcHandlePath = getIPCHandlePath(nonce);
        this.ipcHandlePath = ipcHandlePath;
        try {
            this.server.listen(ipcHandlePath);
            this.server.on('error', err => console.error(err));
        }
        catch (err) {
            console.error('Could not launch git askpass helper.');
            this.enabled = false;
        }
        return ipcHandlePath;
    }
    onRequest(req, res) {
        const chunks = [];
        req.setEncoding('utf8');
        req.on('data', (d) => chunks.push(d));
        req.on('end', () => {
            const { request, host } = JSON.parse(chunks.join(''));
            this.prompt(host, request).then(result => {
                res.writeHead(200);
                res.end(JSON.stringify(result));
            }, () => {
                res.writeHead(500);
                res.end();
            });
        });
    }
    async prompt(host, request) {
        const options = {
            password: /password/i.test(request),
            placeHolder: request,
            prompt: `Git: ${host}`,
            ignoreFocusOut: true
        };
        return await vscode_1.window.showInputBox(options) || '';
    }
    async getEnv() {
        if (!this.enabled) {
            return {
                GIT_ASKPASS: path.join(__dirname, 'askpass-empty.sh')
            };
        }
        return {
            ELECTRON_RUN_AS_NODE: '1',
            GIT_ASKPASS: path.join(__dirname, 'askpass.sh'),
            VSCODE_GIT_ASKPASS_NODE: process.execPath,
            VSCODE_GIT_ASKPASS_MAIN: path.join(__dirname, 'askpass-main.js'),
            VSCODE_GIT_ASKPASS_HANDLE: await this.ipcHandlePathPromise
        };
    }
    dispose() {
        this.server.close();
        if (this.ipcHandlePath && process.platform !== 'win32') {
            fs.unlinkSync(this.ipcHandlePath);
        }
    }
}
exports.Askpass = Askpass;
//# sourceMappingURL=askpass.js.map