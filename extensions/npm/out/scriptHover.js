"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.NpmScriptHoverProvider = exports.invalidateHoverScriptsCache = void 0;
const vscode_1 = require("vscode");
const tasks_1 = require("./tasks");
const nls = require("vscode-nls");
const path_1 = require("path");
const localize = nls.loadMessageBundle();
let cachedDocument = undefined;
let cachedScriptsMap = undefined;
function invalidateHoverScriptsCache(document) {
    if (!document) {
        cachedDocument = undefined;
        return;
    }
    if (document.uri === cachedDocument) {
        cachedDocument = undefined;
    }
}
exports.invalidateHoverScriptsCache = invalidateHoverScriptsCache;
class NpmScriptHoverProvider {
    constructor(context) {
        this.context = context;
        context.subscriptions.push(vscode_1.commands.registerCommand('npm.runScriptFromHover', this.runScriptFromHover, this));
        context.subscriptions.push(vscode_1.commands.registerCommand('npm.debugScriptFromHover', this.debugScriptFromHover, this));
        context.subscriptions.push(vscode_1.workspace.onDidChangeTextDocument((e) => {
            invalidateHoverScriptsCache(e.document);
        }));
    }
    provideHover(document, position, _token) {
        let hover = undefined;
        if (!cachedDocument || cachedDocument.fsPath !== document.uri.fsPath) {
            cachedScriptsMap = tasks_1.findAllScriptRanges(document.getText());
            cachedDocument = document.uri;
        }
        cachedScriptsMap.forEach((value, key) => {
            let start = document.positionAt(value[0]);
            let end = document.positionAt(value[0] + value[1]);
            let range = new vscode_1.Range(start, end);
            if (range.contains(position)) {
                let contents = new vscode_1.MarkdownString();
                contents.isTrusted = true;
                contents.appendMarkdown(this.createRunScriptMarkdown(key, document.uri));
                contents.appendMarkdown(this.createDebugScriptMarkdown(key, document.uri));
                hover = new vscode_1.Hover(contents);
            }
        });
        return hover;
    }
    createRunScriptMarkdown(script, documentUri) {
        let args = {
            documentUri: documentUri,
            script: script,
        };
        return this.createMarkdownLink(localize('runScript', 'Run Script'), 'npm.runScriptFromHover', args, localize('runScript.tooltip', 'Run the script as a task'));
    }
    createDebugScriptMarkdown(script, documentUri) {
        const args = {
            documentUri: documentUri,
            script: script,
        };
        return this.createMarkdownLink(localize('debugScript', 'Debug Script'), 'npm.debugScriptFromHover', args, localize('debugScript.tooltip', 'Runs the script under the debugger'), '|');
    }
    createMarkdownLink(label, cmd, args, tooltip, separator) {
        let encodedArgs = encodeURIComponent(JSON.stringify(args));
        let prefix = '';
        if (separator) {
            prefix = ` ${separator} `;
        }
        return `${prefix}[${label}](command:${cmd}?${encodedArgs} "${tooltip}")`;
    }
    async runScriptFromHover(args) {
        let script = args.script;
        let documentUri = args.documentUri;
        let folder = vscode_1.workspace.getWorkspaceFolder(documentUri);
        if (folder) {
            let task = await tasks_1.createTask(this.context, script, `run ${script}`, folder, documentUri);
            await vscode_1.tasks.executeTask(task);
        }
    }
    debugScriptFromHover(args) {
        let script = args.script;
        let documentUri = args.documentUri;
        let folder = vscode_1.workspace.getWorkspaceFolder(documentUri);
        if (folder) {
            tasks_1.startDebugging(this.context, script, path_1.dirname(documentUri.fsPath), folder);
        }
    }
}
exports.NpmScriptHoverProvider = NpmScriptHoverProvider;
//# sourceMappingURL=scriptHover.js.map