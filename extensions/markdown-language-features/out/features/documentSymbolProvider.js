"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const vscode = require("vscode");
const tableOfContentsProvider_1 = require("../tableOfContentsProvider");
class MDDocumentSymbolProvider {
    constructor(engine) {
        this.engine = engine;
    }
    async provideDocumentSymbolInformation(document) {
        const toc = await new tableOfContentsProvider_1.TableOfContentsProvider(this.engine, document).getToc();
        return toc.map(entry => this.toSymbolInformation(entry));
    }
    async provideDocumentSymbols(document) {
        const toc = await new tableOfContentsProvider_1.TableOfContentsProvider(this.engine, document).getToc();
        const root = {
            level: -Infinity,
            children: [],
            parent: undefined
        };
        this.buildTree(root, toc);
        return root.children;
    }
    buildTree(parent, entries) {
        if (!entries.length) {
            return;
        }
        const entry = entries[0];
        const symbol = this.toDocumentSymbol(entry);
        symbol.children = [];
        while (parent && entry.level <= parent.level) {
            parent = parent.parent;
        }
        parent.children.push(symbol);
        this.buildTree({ level: entry.level, children: symbol.children, parent }, entries.slice(1));
    }
    toSymbolInformation(entry) {
        return new vscode.SymbolInformation(this.getSymbolName(entry), vscode.SymbolKind.String, '', entry.location);
    }
    toDocumentSymbol(entry) {
        return new vscode.DocumentSymbol(this.getSymbolName(entry), '', vscode.SymbolKind.String, entry.location.range, entry.location.range);
    }
    getSymbolName(entry) {
        return '#'.repeat(entry.level) + ' ' + entry.text;
    }
}
exports.default = MDDocumentSymbolProvider;
//# sourceMappingURL=documentSymbolProvider.js.map