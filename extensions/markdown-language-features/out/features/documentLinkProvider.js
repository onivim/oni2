"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const path = require("path");
const vscode = require("vscode");
const nls = require("vscode-nls");
const openDocumentLink_1 = require("../commands/openDocumentLink");
const links_1 = require("../util/links");
const localize = nls.loadMessageBundle();
function parseLink(document, link) {
    const externalSchemeUri = links_1.getUriForLinkWithKnownExternalScheme(link);
    if (externalSchemeUri) {
        // Normalize VS Code links to target currently running version
        if (links_1.isOfScheme(links_1.Schemes.vscode, link) || links_1.isOfScheme(links_1.Schemes['vscode-insiders'], link)) {
            return { uri: vscode.Uri.parse(link).with({ scheme: vscode.env.uriScheme }) };
        }
        return { uri: externalSchemeUri };
    }
    // Assume it must be an relative or absolute file path
    // Use a fake scheme to avoid parse warnings
    const tempUri = vscode.Uri.parse(`vscode-resource:${link}`);
    let resourceUri;
    if (!tempUri.path) {
        resourceUri = document.uri;
    }
    else if (tempUri.path[0] === '/') {
        const root = getWorkspaceFolder(document);
        if (root) {
            resourceUri = vscode.Uri.joinPath(root, tempUri.path);
        }
    }
    else {
        if (document.uri.scheme === links_1.Schemes.untitled) {
            const root = getWorkspaceFolder(document);
            if (root) {
                resourceUri = vscode.Uri.joinPath(root, tempUri.path);
            }
        }
        else {
            const base = document.uri.with({ path: path.dirname(document.uri.fsPath) });
            resourceUri = vscode.Uri.joinPath(base, tempUri.path);
        }
    }
    if (!resourceUri) {
        return undefined;
    }
    resourceUri = resourceUri.with({ fragment: tempUri.fragment });
    return {
        uri: openDocumentLink_1.OpenDocumentLinkCommand.createCommandUri(document.uri, resourceUri, tempUri.fragment),
        tooltip: localize('documentLink.tooltip', 'Follow link')
    };
}
function getWorkspaceFolder(document) {
    var _a, _b, _c;
    return ((_a = vscode.workspace.getWorkspaceFolder(document.uri)) === null || _a === void 0 ? void 0 : _a.uri) || ((_c = (_b = vscode.workspace.workspaceFolders) === null || _b === void 0 ? void 0 : _b[0]) === null || _c === void 0 ? void 0 : _c.uri);
}
function extractDocumentLink(document, pre, link, matchIndex) {
    const offset = (matchIndex || 0) + pre;
    const linkStart = document.positionAt(offset);
    const linkEnd = document.positionAt(offset + link.length);
    try {
        const linkData = parseLink(document, link);
        if (!linkData) {
            return undefined;
        }
        const documentLink = new vscode.DocumentLink(new vscode.Range(linkStart, linkEnd), linkData.uri);
        documentLink.tooltip = linkData.tooltip;
        return documentLink;
    }
    catch (e) {
        return undefined;
    }
}
class LinkProvider {
    constructor() {
        this.linkPattern = /(\[((!\[[^\]]*?\]\(\s*)([^\s\(\)]+?)\s*\)\]|(?:\\\]|[^\]])*\])\(\s*)(([^\s\(\)]|\([^\s\(\)]*?\))+)\s*(".*?")?\)/g;
        this.referenceLinkPattern = /(\[((?:\\\]|[^\]])+)\]\[\s*?)([^\s\]]*?)\]/g;
        this.definitionPattern = /^([\t ]*\[(?!\^)((?:\\\]|[^\]])+)\]:\s*)(\S+)/gm;
    }
    provideDocumentLinks(document, _token) {
        const text = document.getText();
        return [
            ...this.providerInlineLinks(text, document),
            ...this.provideReferenceLinks(text, document)
        ];
    }
    providerInlineLinks(text, document) {
        const results = [];
        for (const match of text.matchAll(this.linkPattern)) {
            const matchImage = match[4] && extractDocumentLink(document, match[3].length + 1, match[4], match.index);
            if (matchImage) {
                results.push(matchImage);
            }
            const matchLink = extractDocumentLink(document, match[1].length, match[5], match.index);
            if (matchLink) {
                results.push(matchLink);
            }
        }
        return results;
    }
    provideReferenceLinks(text, document) {
        const results = [];
        const definitions = this.getDefinitions(text, document);
        for (const match of text.matchAll(this.referenceLinkPattern)) {
            let linkStart;
            let linkEnd;
            let reference = match[3];
            if (reference) { // [text][ref]
                const pre = match[1];
                const offset = (match.index || 0) + pre.length;
                linkStart = document.positionAt(offset);
                linkEnd = document.positionAt(offset + reference.length);
            }
            else if (match[2]) { // [ref][]
                reference = match[2];
                const offset = (match.index || 0) + 1;
                linkStart = document.positionAt(offset);
                linkEnd = document.positionAt(offset + match[2].length);
            }
            else {
                continue;
            }
            try {
                const link = definitions.get(reference);
                if (link) {
                    results.push(new vscode.DocumentLink(new vscode.Range(linkStart, linkEnd), vscode.Uri.parse(`command:_markdown.moveCursorToPosition?${encodeURIComponent(JSON.stringify([link.linkRange.start.line, link.linkRange.start.character]))}`)));
                }
            }
            catch (e) {
                // noop
            }
        }
        for (const definition of definitions.values()) {
            try {
                const linkData = parseLink(document, definition.link);
                if (linkData) {
                    results.push(new vscode.DocumentLink(definition.linkRange, linkData.uri));
                }
            }
            catch (e) {
                // noop
            }
        }
        return results;
    }
    getDefinitions(text, document) {
        const out = new Map();
        for (const match of text.matchAll(this.definitionPattern)) {
            const pre = match[1];
            const reference = match[2];
            const link = match[3].trim();
            const offset = (match.index || 0) + pre.length;
            const linkStart = document.positionAt(offset);
            const linkEnd = document.positionAt(offset + link.length);
            out.set(reference, {
                link: link,
                linkRange: new vscode.Range(linkStart, linkEnd)
            });
        }
        return out;
    }
}
exports.default = LinkProvider;
//# sourceMappingURL=documentLinkProvider.js.map