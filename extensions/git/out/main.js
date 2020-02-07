"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const nls = require("vscode-nls");
const localize = nls.loadMessageBundle();
const vscode_1 = require("vscode");
const git_1 = require("./git");
const model_1 = require("./model");
const commands_1 = require("./commands");
const contentProvider_1 = require("./contentProvider");
const decorationProvider_1 = require("./decorationProvider");
const askpass_1 = require("./askpass");
const util_1 = require("./util");
const vscode_extension_telemetry_1 = require("vscode-extension-telemetry");
const protocolHandler_1 = require("./protocolHandler");
const extension_1 = require("./api/extension");
const path = require("path");
const fs = require("fs");
const deactivateTasks = [];
async function deactivate() {
    for (const task of deactivateTasks) {
        await task();
    }
}
exports.deactivate = deactivate;
async function createModel(context, outputChannel, telemetryReporter, disposables) {
    const pathHint = vscode_1.workspace.getConfiguration('git').get('path');
    const info = await git_1.findGit(pathHint, path => outputChannel.appendLine(localize('looking', "Looking for git in: {0}", path)));
    const askpass = new askpass_1.Askpass();
    disposables.push(askpass);
    const env = await askpass.getEnv();
    const git = new git_1.Git({ gitPath: info.path, version: info.version, env });
    const model = new model_1.Model(git, context.globalState, outputChannel);
    disposables.push(model);
    const onRepository = () => vscode_1.commands.executeCommand('setContext', 'gitOpenRepositoryCount', `${model.repositories.length}`);
    model.onDidOpenRepository(onRepository, null, disposables);
    model.onDidCloseRepository(onRepository, null, disposables);
    onRepository();
    outputChannel.appendLine(localize('using git', "Using git {0} from {1}", info.version, info.path));
    const onOutput = (str) => {
        const lines = str.split(/\r?\n/mg);
        while (/^\s*$/.test(lines[lines.length - 1])) {
            lines.pop();
        }
        outputChannel.appendLine(lines.join('\n'));
    };
    git.onOutput.addListener('log', onOutput);
    disposables.push(util_1.toDisposable(() => git.onOutput.removeListener('log', onOutput)));
    disposables.push(new commands_1.CommandCenter(git, model, outputChannel, telemetryReporter), new contentProvider_1.GitContentProvider(model), new decorationProvider_1.GitDecorations(model), new protocolHandler_1.GitProtocolHandler());
    await checkGitVersion(info);
    return model;
}
async function isGitRepository(folder) {
    if (folder.uri.scheme !== 'file') {
        return false;
    }
    const dotGit = path.join(folder.uri.fsPath, '.git');
    try {
        const dotGitStat = await new Promise((c, e) => fs.stat(dotGit, (err, stat) => err ? e(err) : c(stat)));
        return dotGitStat.isDirectory();
    }
    catch (err) {
        return false;
    }
}
async function warnAboutMissingGit() {
    const config = vscode_1.workspace.getConfiguration('git');
    const shouldIgnore = config.get('ignoreMissingGitWarning') === true;
    if (shouldIgnore) {
        return;
    }
    if (!vscode_1.workspace.workspaceFolders) {
        return;
    }
    const areGitRepositories = await Promise.all(vscode_1.workspace.workspaceFolders.map(isGitRepository));
    if (areGitRepositories.every(isGitRepository => !isGitRepository)) {
        return;
    }
    const download = localize('downloadgit', "Download Git");
    const neverShowAgain = localize('neverShowAgain', "Don't Show Again");
    const choice = await vscode_1.window.showWarningMessage(localize('notfound', "Git not found. Install it or configure it using the 'git.path' setting."), download, neverShowAgain);
    if (choice === download) {
        vscode_1.commands.executeCommand('vscode.open', vscode_1.Uri.parse('https://git-scm.com/'));
    }
    else if (choice === neverShowAgain) {
        await config.update('ignoreMissingGitWarning', true, true);
    }
}
async function activate(context) {
    const disposables = [];
    context.subscriptions.push(new vscode_1.Disposable(() => vscode_1.Disposable.from(...disposables).dispose()));
    const outputChannel = vscode_1.window.createOutputChannel('Git');
    vscode_1.commands.registerCommand('git.showOutput', () => outputChannel.show());
    disposables.push(outputChannel);
    const { name, version, aiKey } = require('../package.json');
    const telemetryReporter = new vscode_extension_telemetry_1.default(name, version, aiKey);
    deactivateTasks.push(() => telemetryReporter.dispose());
    const config = vscode_1.workspace.getConfiguration('git', null);
    const enabled = config.get('enabled');
    if (!enabled) {
        const onConfigChange = util_1.filterEvent(vscode_1.workspace.onDidChangeConfiguration, e => e.affectsConfiguration('git'));
        const onEnabled = util_1.filterEvent(onConfigChange, () => vscode_1.workspace.getConfiguration('git', null).get('enabled') === true);
        const result = new extension_1.GitExtensionImpl();
        util_1.eventToPromise(onEnabled).then(async () => result.model = await createModel(context, outputChannel, telemetryReporter, disposables));
        return result;
    }
    try {
        const model = await createModel(context, outputChannel, telemetryReporter, disposables);
        return new extension_1.GitExtensionImpl(model);
    }
    catch (err) {
        if (!/Git installation not found/.test(err.message || '')) {
            throw err;
        }
        console.warn(err.message);
        outputChannel.appendLine(err.message);
        warnAboutMissingGit();
        return new extension_1.GitExtensionImpl();
    }
}
exports.activate = activate;
async function checkGitVersion(info) {
    const config = vscode_1.workspace.getConfiguration('git');
    const shouldIgnore = config.get('ignoreLegacyWarning') === true;
    if (shouldIgnore) {
        return;
    }
    if (!/^[01]/.test(info.version)) {
        return;
    }
    const update = localize('updateGit', "Update Git");
    const neverShowAgain = localize('neverShowAgain', "Don't Show Again");
    const choice = await vscode_1.window.showWarningMessage(localize('git20', "You seem to have git {0} installed. Code works best with git >= 2", info.version), update, neverShowAgain);
    if (choice === update) {
        vscode_1.commands.executeCommand('vscode.open', vscode_1.Uri.parse('https://git-scm.com/'));
    }
    else if (choice === neverShowAgain) {
        await config.update('ignoreLegacyWarning', true, true);
    }
}
//# sourceMappingURL=main.js.map