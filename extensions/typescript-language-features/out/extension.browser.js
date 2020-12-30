"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = void 0;
const vscode = require("vscode");
const api_1 = require("./api");
const index_1 = require("./commands/index");
const languageConfiguration_1 = require("./languageFeatures/languageConfiguration");
const lazyClientHost_1 = require("./lazyClientHost");
const cancellation_1 = require("./tsServer/cancellation");
const logDirectoryProvider_1 = require("./tsServer/logDirectoryProvider");
const versionProvider_1 = require("./tsServer/versionProvider");
const serverProcess_browser_1 = require("./tsServer/serverProcess.browser");
const api_2 = require("./utils/api");
const commandManager_1 = require("./commands/commandManager");
const plugins_1 = require("./utils/plugins");
class StaticVersionProvider {
    constructor(_version) {
        this._version = _version;
        this.globalVersion = undefined;
        this.localVersion = undefined;
        this.localVersions = [];
    }
    updateConfiguration(_configuration) {
        // noop
    }
    get defaultVersion() { return this._version; }
    get bundledVersion() { return this._version; }
}
function activate(context) {
    const pluginManager = new plugins_1.PluginManager();
    context.subscriptions.push(pluginManager);
    const commandManager = new commandManager_1.CommandManager();
    context.subscriptions.push(commandManager);
    context.subscriptions.push(new languageConfiguration_1.LanguageConfigurationManager());
    const onCompletionAccepted = new vscode.EventEmitter();
    context.subscriptions.push(onCompletionAccepted);
    const versionProvider = new StaticVersionProvider(new versionProvider_1.TypeScriptVersion("bundled" /* Bundled */, vscode.Uri.joinPath(context.extensionUri, 'dist/browser/typescript-web/tsserver.web.js').toString(), api_2.default.fromSimpleString('4.0.3')));
    const lazyClientHost = lazyClientHost_1.createLazyClientHost(context, false, {
        pluginManager,
        commandManager,
        logDirectoryProvider: logDirectoryProvider_1.noopLogDirectoryProvider,
        cancellerFactory: cancellation_1.noopRequestCancellerFactory,
        versionProvider,
        processFactory: serverProcess_browser_1.WorkerServerProcess
    }, item => {
        onCompletionAccepted.fire(item);
    });
    index_1.registerBaseCommands(commandManager, lazyClientHost, pluginManager);
    // context.subscriptions.push(task.register(lazyClientHost.map(x => x.serviceClient)));
    Promise.resolve().then(() => require('./languageFeatures/tsconfig')).then(module => {
        context.subscriptions.push(module.register());
    });
    context.subscriptions.push(lazyClientHost_1.lazilyActivateClient(lazyClientHost, pluginManager));
    return api_1.getExtensionApi(onCompletionAccepted.event, pluginManager);
}
exports.activate = activate;
//# sourceMappingURL=extension.browser.js.map