"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const assert = require("assert");
const vscode = require("vscode");
require("mocha");
const inMemoryDocument_1 = require("./inMemoryDocument");
const engine_1 = require("./engine");
const testFileName = vscode.Uri.file('test.md');
suite('markdown.engine', () => {
    suite('rendering', () => {
        const input = '# hello\n\nworld!';
        const output = '<h1 id="hello" data-line="0" class="code-line">hello</h1>\n'
            + '<p data-line="2" class="code-line">world!</p>\n';
        test('Renders a document', async () => {
            const doc = new inMemoryDocument_1.InMemoryDocument(testFileName, input);
            const engine = engine_1.createNewMarkdownEngine();
            assert.strictEqual(await engine.render(doc), output);
        });
        test('Renders a string', async () => {
            const engine = engine_1.createNewMarkdownEngine();
            assert.strictEqual(await engine.render(input), output);
        });
    });
});
//# sourceMappingURL=engine.test.js.map