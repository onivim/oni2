"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.deactivate = exports.activate = void 0;
const httpRequest = require("request-light");
const vscode = require("vscode");
const jsonContributions_1 = require("./features/jsonContributions");
const commands_1 = require("./commands");
const npmView_1 = require("./npmView");
const tasks_1 = require("./tasks");
const scriptHover_1 = require("./scriptHover");
let treeDataProvider;
function invalidateScriptCaches() {
    scriptHover_1.invalidateHoverScriptsCache();
    tasks_1.invalidateTasksCache();
    if (treeDataProvider) {
        treeDataProvider.refresh();
    }
}
async function activate(context) {
    configureHttpRequest();
    context.subscriptions.push(vscode.workspace.onDidChangeConfiguration(e => {
        if (e.affectsConfiguration('http.proxy') || e.affectsConfiguration('http.proxyStrictSSL')) {
            configureHttpRequest();
        }
    }));
    const canRunNPM = canRunNpmInCurrentWorkspace();
    context.subscriptions.push(jsonContributions_1.addJSONProviders(httpRequest.xhr, canRunNPM));
    registerTaskProvider(context);
    treeDataProvider = registerExplorer(context);
    context.subscriptions.push(vscode.workspace.onDidChangeConfiguration((e) => {
        if (e.affectsConfiguration('npm.exclude') || e.affectsConfiguration('npm.autoDetect')) {
            tasks_1.invalidateTasksCache();
            if (treeDataProvider) {
                treeDataProvider.refresh();
            }
        }
        if (e.affectsConfiguration('npm.scriptExplorerAction')) {
            if (treeDataProvider) {
                treeDataProvider.refresh();
            }
        }
    }));
    registerHoverProvider(context);
    context.subscriptions.push(vscode.commands.registerCommand('npm.runSelectedScript', commands_1.runSelectedScript));
    context.subscriptions.push(vscode.commands.registerCommand('npm.runScriptFromFolder', commands_1.selectAndRunScriptFromFolder));
    context.subscriptions.push(vscode.commands.registerCommand('npm.refresh', () => {
        invalidateScriptCaches();
    }));
    context.subscriptions.push(vscode.commands.registerCommand('npm.packageManager', (args) => {
        if (args instanceof vscode.Uri) {
            return tasks_1.getPackageManager(context, args);
        }
        return '';
    }));
}
exports.activate = activate;
function canRunNpmInCurrentWorkspace() {
    if (vscode.workspace.workspaceFolders) {
        return vscode.workspace.workspaceFolders.some(f => f.uri.scheme === 'file');
    }
    return false;
}
let taskProvider;
function registerTaskProvider(context) {
    if (vscode.workspace.workspaceFolders) {
        let watcher = vscode.workspace.createFileSystemWatcher('**/package.json');
        watcher.onDidChange((_e) => invalidateScriptCaches());
        watcher.onDidDelete((_e) => invalidateScriptCaches());
        watcher.onDidCreate((_e) => invalidateScriptCaches());
        context.subscriptions.push(watcher);
        let workspaceWatcher = vscode.workspace.onDidChangeWorkspaceFolders((_e) => invalidateScriptCaches());
        context.subscriptions.push(workspaceWatcher);
        taskProvider = new tasks_1.NpmTaskProvider(context);
        let disposable = vscode.tasks.registerTaskProvider('npm', taskProvider);
        context.subscriptions.push(disposable);
        return disposable;
    }
    return undefined;
}
function registerExplorer(context) {
    if (vscode.workspace.workspaceFolders) {
        let treeDataProvider = new npmView_1.NpmScriptsTreeDataProvider(context, taskProvider);
        const view = vscode.window.createTreeView('npm', { treeDataProvider: treeDataProvider, showCollapseAll: true });
        context.subscriptions.push(view);
        return treeDataProvider;
    }
    return undefined;
}
function registerHoverProvider(context) {
    if (vscode.workspace.workspaceFolders) {
        let npmSelector = {
            language: 'json',
            scheme: 'file',
            pattern: '**/package.json'
        };
        let provider = new scriptHover_1.NpmScriptHoverProvider(context);
        context.subscriptions.push(vscode.languages.registerHoverProvider(npmSelector, provider));
        return provider;
    }
    return undefined;
}
function configureHttpRequest() {
    const httpSettings = vscode.workspace.getConfiguration('http');
    httpRequest.configure(httpSettings.get('proxy', ''), httpSettings.get('proxyStrictSSL', true));
}
function deactivate() {
}
exports.deactivate = deactivate;
//# sourceMappingURL=npmMain.js.map