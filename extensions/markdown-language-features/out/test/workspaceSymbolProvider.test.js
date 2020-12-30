"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const assert = require("assert");
require("mocha");
const vscode = require("vscode");
const documentSymbolProvider_1 = require("../features/documentSymbolProvider");
const workspaceSymbolProvider_1 = require("../features/workspaceSymbolProvider");
const engine_1 = require("./engine");
const inMemoryDocument_1 = require("./inMemoryDocument");
const symbolProvider = new documentSymbolProvider_1.default(engine_1.createNewMarkdownEngine());
suite('markdown.WorkspaceSymbolProvider', () => {
    test('Should not return anything for empty workspace', async () => {
        const provider = new workspaceSymbolProvider_1.default(symbolProvider, new InMemoryWorkspaceMarkdownDocumentProvider([]));
        assert.deepEqual(await provider.provideWorkspaceSymbols(''), []);
    });
    test('Should return symbols from workspace with one markdown file', async () => {
        const testFileName = vscode.Uri.file('test.md');
        const provider = new workspaceSymbolProvider_1.default(symbolProvider, new InMemoryWorkspaceMarkdownDocumentProvider([
            new inMemoryDocument_1.InMemoryDocument(testFileName, `# header1\nabc\n## header2`)
        ]));
        const symbols = await provider.provideWorkspaceSymbols('');
        assert.strictEqual(symbols.length, 2);
        assert.strictEqual(symbols[0].name, '# header1');
        assert.strictEqual(symbols[1].name, '## header2');
    });
    test('Should return all content  basic workspace', async () => {
        const fileNameCount = 10;
        const files = [];
        for (let i = 0; i < fileNameCount; ++i) {
            const testFileName = vscode.Uri.file(`test${i}.md`);
            files.push(new inMemoryDocument_1.InMemoryDocument(testFileName, `# common\nabc\n## header${i}`));
        }
        const provider = new workspaceSymbolProvider_1.default(symbolProvider, new InMemoryWorkspaceMarkdownDocumentProvider(files));
        const symbols = await provider.provideWorkspaceSymbols('');
        assert.strictEqual(symbols.length, fileNameCount * 2);
    });
    test('Should update results when markdown file changes symbols', async () => {
        const testFileName = vscode.Uri.file('test.md');
        const workspaceFileProvider = new InMemoryWorkspaceMarkdownDocumentProvider([
            new inMemoryDocument_1.InMemoryDocument(testFileName, `# header1`, 1 /* version */)
        ]);
        const provider = new workspaceSymbolProvider_1.default(symbolProvider, workspaceFileProvider);
        assert.strictEqual((await provider.provideWorkspaceSymbols('')).length, 1);
        // Update file
        workspaceFileProvider.updateDocument(new inMemoryDocument_1.InMemoryDocument(testFileName, `# new header\nabc\n## header2`, 2 /* version */));
        const newSymbols = await provider.provideWorkspaceSymbols('');
        assert.strictEqual(newSymbols.length, 2);
        assert.strictEqual(newSymbols[0].name, '# new header');
        assert.strictEqual(newSymbols[1].name, '## header2');
    });
    test('Should remove results when file is deleted', async () => {
        const testFileName = vscode.Uri.file('test.md');
        const workspaceFileProvider = new InMemoryWorkspaceMarkdownDocumentProvider([
            new inMemoryDocument_1.InMemoryDocument(testFileName, `# header1`)
        ]);
        const provider = new workspaceSymbolProvider_1.default(symbolProvider, workspaceFileProvider);
        assert.strictEqual((await provider.provideWorkspaceSymbols('')).length, 1);
        // delete file
        workspaceFileProvider.deleteDocument(testFileName);
        const newSymbols = await provider.provideWorkspaceSymbols('');
        assert.strictEqual(newSymbols.length, 0);
    });
    test('Should update results when markdown file is created', async () => {
        const testFileName = vscode.Uri.file('test.md');
        const workspaceFileProvider = new InMemoryWorkspaceMarkdownDocumentProvider([
            new inMemoryDocument_1.InMemoryDocument(testFileName, `# header1`)
        ]);
        const provider = new workspaceSymbolProvider_1.default(symbolProvider, workspaceFileProvider);
        assert.strictEqual((await provider.provideWorkspaceSymbols('')).length, 1);
        // Creat file
        workspaceFileProvider.createDocument(new inMemoryDocument_1.InMemoryDocument(vscode.Uri.file('test2.md'), `# new header\nabc\n## header2`));
        const newSymbols = await provider.provideWorkspaceSymbols('');
        assert.strictEqual(newSymbols.length, 3);
    });
});
class InMemoryWorkspaceMarkdownDocumentProvider {
    constructor(documents) {
        this._documents = new Map();
        this._onDidChangeMarkdownDocumentEmitter = new vscode.EventEmitter();
        this.onDidChangeMarkdownDocument = this._onDidChangeMarkdownDocumentEmitter.event;
        this._onDidCreateMarkdownDocumentEmitter = new vscode.EventEmitter();
        this.onDidCreateMarkdownDocument = this._onDidCreateMarkdownDocumentEmitter.event;
        this._onDidDeleteMarkdownDocumentEmitter = new vscode.EventEmitter();
        this.onDidDeleteMarkdownDocument = this._onDidDeleteMarkdownDocumentEmitter.event;
        for (const doc of documents) {
            this._documents.set(doc.fileName, doc);
        }
    }
    async getAllMarkdownDocuments() {
        return Array.from(this._documents.values());
    }
    updateDocument(document) {
        this._documents.set(document.fileName, document);
        this._onDidChangeMarkdownDocumentEmitter.fire(document);
    }
    createDocument(document) {
        assert.ok(!this._documents.has(document.uri.fsPath));
        this._documents.set(document.uri.fsPath, document);
        this._onDidCreateMarkdownDocumentEmitter.fire(document);
    }
    deleteDocument(resource) {
        this._documents.delete(resource.fsPath);
        this._onDidDeleteMarkdownDocumentEmitter.fire(resource);
    }
}
//# sourceMappingURL=workspaceSymbolProvider.test.js.map