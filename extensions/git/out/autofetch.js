"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const vscode_1 = require("vscode");
const util_1 = require("./util");
const nls = require("vscode-nls");
const localize = nls.loadMessageBundle();
function isRemoteOperation(operation) {
    return operation === "Pull" /* Pull */ || operation === "Push" /* Push */ || operation === "Sync" /* Sync */ || operation === "Fetch" /* Fetch */;
}
class AutoFetcher {
    constructor(repository, globalState) {
        this.repository = repository;
        this.globalState = globalState;
        this._onDidChange = new vscode_1.EventEmitter();
        this.onDidChange = this._onDidChange.event;
        this._enabled = false;
        this.disposables = [];
        vscode_1.workspace.onDidChangeConfiguration(this.onConfiguration, this, this.disposables);
        this.onConfiguration();
        const onGoodRemoteOperation = util_1.filterEvent(repository.onDidRunOperation, ({ operation, error }) => !error && isRemoteOperation(operation));
        const onFirstGoodRemoteOperation = util_1.onceEvent(onGoodRemoteOperation);
        onFirstGoodRemoteOperation(this.onFirstGoodRemoteOperation, this, this.disposables);
    }
    get enabled() { return this._enabled; }
    set enabled(enabled) { this._enabled = enabled; this._onDidChange.fire(enabled); }
    async onFirstGoodRemoteOperation() {
        const didInformUser = !this.globalState.get(AutoFetcher.DidInformUser);
        if (this.enabled && !didInformUser) {
            this.globalState.update(AutoFetcher.DidInformUser, true);
        }
        const shouldInformUser = !this.enabled && didInformUser;
        if (!shouldInformUser) {
            return;
        }
        const yes = { title: localize('yes', "Yes") };
        const no = { isCloseAffordance: true, title: localize('no', "No") };
        const askLater = { title: localize('not now', "Ask Me Later") };
        const result = await vscode_1.window.showInformationMessage(localize('suggest auto fetch', "Would you like Code to [periodically run 'git fetch']({0})?", 'https://go.microsoft.com/fwlink/?linkid=865294'), yes, no, askLater);
        if (result === askLater) {
            return;
        }
        if (result === yes) {
            const gitConfig = vscode_1.workspace.getConfiguration('git', vscode_1.Uri.file(this.repository.root));
            gitConfig.update('autofetch', true, vscode_1.ConfigurationTarget.Global);
        }
        this.globalState.update(AutoFetcher.DidInformUser, true);
    }
    onConfiguration() {
        const gitConfig = vscode_1.workspace.getConfiguration('git', vscode_1.Uri.file(this.repository.root));
        if (gitConfig.get('autofetch') === false) {
            this.disable();
        }
        else {
            this.enable();
        }
    }
    enable() {
        if (this.enabled) {
            return;
        }
        this.enabled = true;
        this.run();
    }
    disable() {
        this.enabled = false;
    }
    async run() {
        while (this.enabled) {
            await this.repository.whenIdleAndFocused();
            if (!this.enabled) {
                return;
            }
            try {
                await this.repository.fetchDefault();
            }
            catch (err) {
                if (err.gitErrorCode === "AuthenticationFailed" /* AuthenticationFailed */) {
                    this.disable();
                }
            }
            if (!this.enabled) {
                return;
            }
            const period = vscode_1.workspace.getConfiguration('git', vscode_1.Uri.file(this.repository.root)).get('autofetchPeriod', 180) * 1000;
            const timeout = new Promise(c => setTimeout(c, period));
            const whenDisabled = util_1.eventToPromise(util_1.filterEvent(this.onDidChange, enabled => !enabled));
            await Promise.race([timeout, whenDisabled]);
        }
    }
    dispose() {
        this.disable();
        this.disposables.forEach(d => d.dispose());
    }
}
AutoFetcher.DidInformUser = 'autofetch.didInformUser';
exports.AutoFetcher = AutoFetcher;
//# sourceMappingURL=autofetch.js.map