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
const inMemoryDocument_1 = require("./inMemoryDocument");
const engine_1 = require("./engine");
const testFileName = vscode.Uri.file('test.md');
function getSymbolsForFile(fileContents) {
    const doc = new inMemoryDocument_1.InMemoryDocument(testFileName, fileContents);
    const provider = new documentSymbolProvider_1.default(engine_1.createNewMarkdownEngine());
    return provider.provideDocumentSymbols(doc);
}
suite('markdown.DocumentSymbolProvider', () => {
    test('Should not return anything for empty document', async () => {
        const symbols = await getSymbolsForFile('');
        assert.strictEqual(symbols.length, 0);
    });
    test('Should not return anything for document with no headers', async () => {
        const symbols = await getSymbolsForFile('a\na');
        assert.strictEqual(symbols.length, 0);
    });
    test('Should not return anything for document with # but no real headers', async () => {
        const symbols = await getSymbolsForFile('a#a\na#');
        assert.strictEqual(symbols.length, 0);
    });
    test('Should return single symbol for single header', async () => {
        const symbols = await getSymbolsForFile('# h');
        assert.strictEqual(symbols.length, 1);
        assert.strictEqual(symbols[0].name, '# h');
    });
    test('Should not care about symbol level for single header', async () => {
        const symbols = await getSymbolsForFile('### h');
        assert.strictEqual(symbols.length, 1);
        assert.strictEqual(symbols[0].name, '### h');
    });
    test('Should put symbols of same level in flat list', async () => {
        const symbols = await getSymbolsForFile('## h\n## h2');
        assert.strictEqual(symbols.length, 2);
        assert.strictEqual(symbols[0].name, '## h');
        assert.strictEqual(symbols[1].name, '## h2');
    });
    test('Should nest symbol of level - 1 under parent', async () => {
        const symbols = await getSymbolsForFile('# h\n## h2\n## h3');
        assert.strictEqual(symbols.length, 1);
        assert.strictEqual(symbols[0].name, '# h');
        assert.strictEqual(symbols[0].children.length, 2);
        assert.strictEqual(symbols[0].children[0].name, '## h2');
        assert.strictEqual(symbols[0].children[1].name, '## h3');
    });
    test('Should nest symbol of level - n under parent', async () => {
        const symbols = await getSymbolsForFile('# h\n#### h2');
        assert.strictEqual(symbols.length, 1);
        assert.strictEqual(symbols[0].name, '# h');
        assert.strictEqual(symbols[0].children.length, 1);
        assert.strictEqual(symbols[0].children[0].name, '#### h2');
    });
    test('Should flatten children where lower level occurs first', async () => {
        const symbols = await getSymbolsForFile('# h\n### h2\n## h3');
        assert.strictEqual(symbols.length, 1);
        assert.strictEqual(symbols[0].name, '# h');
        assert.strictEqual(symbols[0].children.length, 2);
        assert.strictEqual(symbols[0].children[0].name, '### h2');
        assert.strictEqual(symbols[0].children[1].name, '## h3');
    });
    test('Should handle line separator in file. Issue #63749', async () => {
        const symbols = await getSymbolsForFile(`# A
- foo 

# B
- bar`);
        assert.strictEqual(symbols.length, 2);
        assert.strictEqual(symbols[0].name, '# A');
        assert.strictEqual(symbols[1].name, '# B');
    });
});
//# sourceMappingURL=documentSymbolProvider.test.js.map