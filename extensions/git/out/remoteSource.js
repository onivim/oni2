"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.pickRemoteSource = void 0;
const vscode_1 = require("vscode");
const nls = require("vscode-nls");
const decorators_1 = require("./decorators");
const localize = nls.loadMessageBundle();
async function getQuickPickResult(quickpick) {
    const result = await new Promise(c => {
        quickpick.onDidAccept(() => c(quickpick.selectedItems[0]));
        quickpick.onDidHide(() => c(undefined));
        quickpick.show();
    });
    quickpick.hide();
    return result;
}
class RemoteSourceProviderQuickPick {
    constructor(provider) {
        this.provider = provider;
        this.quickpick = vscode_1.window.createQuickPick();
        this.quickpick.ignoreFocusOut = true;
        if (provider.supportsQuery) {
            this.quickpick.placeholder = localize('type to search', "Repository name (type to search)");
            this.quickpick.onDidChangeValue(this.onDidChangeValue, this);
        }
        else {
            this.quickpick.placeholder = localize('type to filter', "Repository name");
        }
    }
    onDidChangeValue() {
        this.query();
    }
    async query() {
        this.quickpick.busy = true;
        try {
            const remoteSources = await this.provider.getRemoteSources(this.quickpick.value) || [];
            if (remoteSources.length === 0) {
                this.quickpick.items = [{
                        label: localize('none found', "No remote repositories found."),
                        alwaysShow: true
                    }];
            }
            else {
                this.quickpick.items = remoteSources.map(remoteSource => ({
                    label: remoteSource.name,
                    description: remoteSource.description || (typeof remoteSource.url === 'string' ? remoteSource.url : remoteSource.url[0]),
                    remoteSource,
                    alwaysShow: true
                }));
            }
        }
        catch (err) {
            this.quickpick.items = [{ label: localize('error', "$(error) Error: {0}", err.message), alwaysShow: true }];
            console.error(err);
        }
        finally {
            this.quickpick.busy = false;
        }
    }
    async pick() {
        this.query();
        const result = await getQuickPickResult(this.quickpick);
        return result === null || result === void 0 ? void 0 : result.remoteSource;
    }
}
__decorate([
    decorators_1.debounce(300)
], RemoteSourceProviderQuickPick.prototype, "onDidChangeValue", null);
__decorate([
    decorators_1.throttle
], RemoteSourceProviderQuickPick.prototype, "query", null);
async function pickRemoteSource(model, options = {}) {
    const quickpick = vscode_1.window.createQuickPick();
    quickpick.ignoreFocusOut = true;
    if (options.providerName) {
        const provider = model.getRemoteProviders()
            .filter(provider => provider.name === options.providerName)[0];
        if (provider) {
            return await pickProviderSource(provider, options);
        }
    }
    const providers = model.getRemoteProviders()
        .map(provider => ({ label: (provider.icon ? `$(${provider.icon}) ` : '') + (options.providerLabel ? options.providerLabel(provider) : provider.name), alwaysShow: true, provider }));
    quickpick.placeholder = providers.length === 0
        ? localize('provide url', "Provide repository URL")
        : localize('provide url or pick', "Provide repository URL or pick a repository source.");
    const updatePicks = (value) => {
        var _a;
        if (value) {
            quickpick.items = [{
                    label: (_a = options.urlLabel) !== null && _a !== void 0 ? _a : localize('url', "URL"),
                    description: value,
                    alwaysShow: true,
                    url: value
                }, ...providers];
        }
        else {
            quickpick.items = providers;
        }
    };
    quickpick.onDidChangeValue(updatePicks);
    updatePicks();
    const result = await getQuickPickResult(quickpick);
    if (result) {
        if (result.url) {
            return result.url;
        }
        else if (result.provider) {
            return await pickProviderSource(result.provider, options);
        }
    }
    return undefined;
}
exports.pickRemoteSource = pickRemoteSource;
async function pickProviderSource(provider, options = {}) {
    const quickpick = new RemoteSourceProviderQuickPick(provider);
    const remote = await quickpick.pick();
    let url;
    if (remote) {
        if (typeof remote.url === 'string') {
            url = remote.url;
        }
        else if (remote.url.length > 0) {
            url = await vscode_1.window.showQuickPick(remote.url, { ignoreFocusOut: true, placeHolder: localize('pick url', "Choose a URL to clone from.") });
        }
    }
    if (!url || !options.branch) {
        return url;
    }
    if (!provider.getBranches) {
        return { url };
    }
    const branches = await provider.getBranches(url);
    if (!branches) {
        return { url };
    }
    const branch = await vscode_1.window.showQuickPick(branches, {
        placeHolder: localize('branch name', "Branch name")
    });
    if (!branch) {
        return { url };
    }
    return { url, branch };
}
//# sourceMappingURL=remoteSource.js.map