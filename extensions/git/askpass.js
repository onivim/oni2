"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.Askpass = void 0;
const vscode_1 = require("vscode");
const util_1 = require("./util");
const path = require("path");
const ipcServer_1 = require("./ipc/ipcServer");
class Askpass {
    constructor(ipc) {
        this.ipc = ipc;
        this.disposable = util_1.EmptyDisposable;
        this.cache = new Map();
        this.credentialsProviders = new Set();
        if (ipc) {
            this.disposable = ipc.registerHandler('askpass', this);
        }
    }
    static async create(outputChannel, context) {
        try {
            return new Askpass(await ipcServer_1.createIPCServer(context));
        }
        catch (err) {
            outputChannel.appendLine(`[error] Failed to create git askpass IPC: ${err}`);
            return new Askpass();
        }
    }
    async handle({ request, host }) {
        const config = vscode_1.workspace.getConfiguration('git', null);
        const enabled = config.get('enabled');
        if (!enabled) {
            return '';
        }
        const uri = vscode_1.Uri.parse(host);
        const authority = uri.authority.replace(/^.*@/, '');
        const password = /password/i.test(request);
        const cached = this.cache.get(authority);
        if (cached && password) {
            this.cache.delete(authority);
            return cached.password;
        }
        if (!password) {
            for (const credentialsProvider of this.credentialsProviders) {
                try {
                    const credentials = await credentialsProvider.getCredentials(uri);
                    if (credentials) {
                        this.cache.set(authority, credentials);
                        setTimeout(() => this.cache.delete(authority), 60000);
                        return credentials.username;
                    }
                }
                catch (_a) { }
            }
        }
        const options = {
            password,
            placeHolder: request,
            prompt: `Git: ${host}`,
            ignoreFocusOut: true
        };
        return await vscode_1.window.showInputBox(options) || '';
    }
    getEnv() {
        if (!this.ipc) {
            return {
                GIT_ASKPASS: path.join(__dirname, 'askpass-empty.sh')
            };
        }
        return {
            ...this.ipc.getEnv(),
            GIT_ASKPASS: path.join(__dirname, 'askpass.sh'),
            VSCODE_GIT_ASKPASS_NODE: process.execPath,
            VSCODE_GIT_ASKPASS_MAIN: path.join(__dirname, 'askpass-main.js')
        };
    }
    registerCredentialsProvider(provider) {
        this.credentialsProviders.add(provider);
        return util_1.toDisposable(() => this.credentialsProviders.delete(provider));
    }
    dispose() {
        this.disposable.dispose();
    }
}
exports.Askpass = Askpass;
//# sourceMappingURL=askpass.js.map