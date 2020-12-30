"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.resolveLinkToMarkdownFile = exports.OpenDocumentLinkCommand = void 0;
const vscode = require("vscode");
const path_1 = require("path");
const tableOfContentsProvider_1 = require("../tableOfContentsProvider");
const file_1 = require("../util/file");
var OpenMarkdownLinks;
(function (OpenMarkdownLinks) {
    OpenMarkdownLinks["beside"] = "beside";
    OpenMarkdownLinks["currentGroup"] = "currentGroup";
})(OpenMarkdownLinks || (OpenMarkdownLinks = {}));
class OpenDocumentLinkCommand {
    constructor(engine) {
        this.engine = engine;
        this.id = OpenDocumentLinkCommand.id;
    }
    static createCommandUri(fromResource, path, fragment) {
        const toJson = (uri) => {
            return {
                scheme: uri.scheme,
                authority: uri.authority,
                path: uri.path,
                fragment: uri.fragment,
                query: uri.query,
            };
        };
        return vscode.Uri.parse(`command:${OpenDocumentLinkCommand.id}?${encodeURIComponent(JSON.stringify({
            parts: toJson(path),
            fragment,
            fromResource: toJson(fromResource),
        }))}`);
    }
    async execute(args) {
        return OpenDocumentLinkCommand.execute(this.engine, args);
    }
    static async execute(engine, args) {
        const fromResource = vscode.Uri.parse('').with(args.fromResource);
        const targetResource = reviveUri(args.parts);
        const column = this.getViewColumn(fromResource);
        const didOpen = await this.tryOpen(engine, targetResource, args, column);
        if (didOpen) {
            return;
        }
        if (path_1.extname(targetResource.path) === '') {
            await this.tryOpen(engine, targetResource.with({ path: targetResource.path + '.md' }), args, column);
            return;
        }
    }
    static async tryOpen(engine, resource, args, column) {
        const tryUpdateForActiveFile = async () => {
            if (vscode.window.activeTextEditor && file_1.isMarkdownFile(vscode.window.activeTextEditor.document)) {
                if (vscode.window.activeTextEditor.document.uri.fsPath === resource.fsPath) {
                    await this.tryRevealLine(engine, vscode.window.activeTextEditor, args.fragment);
                    return true;
                }
            }
            return false;
        };
        if (await tryUpdateForActiveFile()) {
            return true;
        }
        let stat;
        try {
            stat = await vscode.workspace.fs.stat(resource);
        }
        catch (_a) {
            return false;
        }
        if (stat.type === vscode.FileType.Directory) {
            await vscode.commands.executeCommand('revealInExplorer', resource);
            return true;
        }
        try {
            await vscode.commands.executeCommand('vscode.open', resource, column);
        }
        catch (_b) {
            return false;
        }
        return tryUpdateForActiveFile();
    }
    static getViewColumn(resource) {
        const config = vscode.workspace.getConfiguration('markdown', resource);
        const openLinks = config.get('links.openLocation', OpenMarkdownLinks.currentGroup);
        switch (openLinks) {
            case OpenMarkdownLinks.beside:
                return vscode.ViewColumn.Beside;
            case OpenMarkdownLinks.currentGroup:
            default:
                return vscode.ViewColumn.Active;
        }
    }
    static async tryRevealLine(engine, editor, fragment) {
        if (fragment) {
            const toc = new tableOfContentsProvider_1.TableOfContentsProvider(engine, editor.document);
            const entry = await toc.lookup(fragment);
            if (entry) {
                const lineStart = new vscode.Range(entry.line, 0, entry.line, 0);
                editor.selection = new vscode.Selection(lineStart.start, lineStart.end);
                return editor.revealRange(lineStart, vscode.TextEditorRevealType.AtTop);
            }
            const lineNumberFragment = fragment.match(/^L(\d+)$/i);
            if (lineNumberFragment) {
                const line = +lineNumberFragment[1] - 1;
                if (!isNaN(line)) {
                    const lineStart = new vscode.Range(line, 0, line, 0);
                    editor.selection = new vscode.Selection(lineStart.start, lineStart.end);
                    return editor.revealRange(lineStart, vscode.TextEditorRevealType.AtTop);
                }
            }
        }
    }
}
exports.OpenDocumentLinkCommand = OpenDocumentLinkCommand;
OpenDocumentLinkCommand.id = '_markdown.openDocumentLink';
function reviveUri(parts) {
    if (parts.scheme === 'file') {
        return vscode.Uri.file(parts.path);
    }
    return vscode.Uri.parse('').with(parts);
}
async function resolveLinkToMarkdownFile(path) {
    try {
        const standardLink = await tryResolveLinkToMarkdownFile(path);
        if (standardLink) {
            return standardLink;
        }
    }
    catch (_a) {
        // Noop
    }
    // If no extension, try with `.md` extension
    if (path_1.extname(path) === '') {
        return tryResolveLinkToMarkdownFile(path + '.md');
    }
    return undefined;
}
exports.resolveLinkToMarkdownFile = resolveLinkToMarkdownFile;
async function tryResolveLinkToMarkdownFile(path) {
    const resource = vscode.Uri.file(path);
    let document;
    try {
        document = await vscode.workspace.openTextDocument(resource);
    }
    catch (_a) {
        return undefined;
    }
    if (file_1.isMarkdownFile(document)) {
        return document.uri;
    }
    return undefined;
}
//# sourceMappingURL=openDocumentLink.js.map