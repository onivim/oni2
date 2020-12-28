"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
require("mocha");
const assert = require("assert");
const path = require("path");
const vscode_uri_1 = require("vscode-uri");
const vscode_languageserver_types_1 = require("vscode-languageserver-types");
const vscode_css_languageservice_1 = require("vscode-css-languageservice");
const nodeFs_1 = require("../node/nodeFs");
const documentContext_1 = require("../utils/documentContext");
suite('Completions', () => {
    let assertCompletion = function (completions, expected, document, _offset) {
        let matches = completions.items.filter(completion => {
            return completion.label === expected.label;
        });
        assert.equal(matches.length, 1, `${expected.label} should only existing once: Actual: ${completions.items.map(c => c.label).join(', ')}`);
        let match = matches[0];
        if (expected.resultText && vscode_languageserver_types_1.TextEdit.is(match.textEdit)) {
            assert.equal(vscode_languageserver_types_1.TextDocument.applyEdits(document, [match.textEdit]), expected.resultText);
        }
    };
    async function assertCompletions(value, expected, testUri, workspaceFolders, lang = 'css') {
        const offset = value.indexOf('|');
        value = value.substr(0, offset) + value.substr(offset + 1);
        const document = vscode_languageserver_types_1.TextDocument.create(testUri, lang, 0, value);
        const position = document.positionAt(offset);
        if (!workspaceFolders) {
            workspaceFolders = [{ name: 'x', uri: testUri.substr(0, testUri.lastIndexOf('/')) }];
        }
        const lsOptions = { fileSystemProvider: nodeFs_1.getNodeFSRequestService() };
        const cssLanguageService = lang === 'scss' ? vscode_css_languageservice_1.getSCSSLanguageService(lsOptions) : vscode_css_languageservice_1.getCSSLanguageService(lsOptions);
        const context = documentContext_1.getDocumentContext(testUri, workspaceFolders);
        const stylesheet = cssLanguageService.parseStylesheet(document);
        let list = await cssLanguageService.doComplete2(document, position, stylesheet, context);
        if (expected.count) {
            assert.equal(list.items.length, expected.count);
        }
        if (expected.items) {
            for (let item of expected.items) {
                assertCompletion(list, item, document, offset);
            }
        }
    }
    test('CSS url() Path completion', async function () {
        let testUri = vscode_uri_1.URI.file(path.resolve(__dirname, '../../test/pathCompletionFixtures/about/about.css')).toString();
        let folders = [{ name: 'x', uri: vscode_uri_1.URI.file(path.resolve(__dirname, '../../test')).toString() }];
        await assertCompletions('html { background-image: url("./|")', {
            items: [
                { label: 'about.html', resultText: 'html { background-image: url("./about.html")' }
            ]
        }, testUri, folders);
        await assertCompletions(`html { background-image: url('../|')`, {
            items: [
                { label: 'about/', resultText: `html { background-image: url('../about/')` },
                { label: 'index.html', resultText: `html { background-image: url('../index.html')` },
                { label: 'src/', resultText: `html { background-image: url('../src/')` }
            ]
        }, testUri, folders);
        await assertCompletions(`html { background-image: url('../src/a|')`, {
            items: [
                { label: 'feature.js', resultText: `html { background-image: url('../src/feature.js')` },
                { label: 'data/', resultText: `html { background-image: url('../src/data/')` },
                { label: 'test.js', resultText: `html { background-image: url('../src/test.js')` }
            ]
        }, testUri, folders);
        await assertCompletions(`html { background-image: url('../src/data/f|.asar')`, {
            items: [
                { label: 'foo.asar', resultText: `html { background-image: url('../src/data/foo.asar')` }
            ]
        }, testUri, folders);
        await assertCompletions(`html { background-image: url('|')`, {
            items: [
                { label: 'about.html', resultText: `html { background-image: url('about.html')` },
            ]
        }, testUri, folders);
        await assertCompletions(`html { background-image: url('/|')`, {
            items: [
                { label: 'pathCompletionFixtures/', resultText: `html { background-image: url('/pathCompletionFixtures/')` }
            ]
        }, testUri, folders);
        await assertCompletions(`html { background-image: url('/pathCompletionFixtures/|')`, {
            items: [
                { label: 'about/', resultText: `html { background-image: url('/pathCompletionFixtures/about/')` },
                { label: 'index.html', resultText: `html { background-image: url('/pathCompletionFixtures/index.html')` },
                { label: 'src/', resultText: `html { background-image: url('/pathCompletionFixtures/src/')` }
            ]
        }, testUri, folders);
        await assertCompletions(`html { background-image: url("/|")`, {
            items: [
                { label: 'pathCompletionFixtures/', resultText: `html { background-image: url("/pathCompletionFixtures/")` }
            ]
        }, testUri, folders);
    });
    test('CSS url() Path Completion - Unquoted url', async function () {
        let testUri = vscode_uri_1.URI.file(path.resolve(__dirname, '../../test/pathCompletionFixtures/about/about.css')).toString();
        let folders = [{ name: 'x', uri: vscode_uri_1.URI.file(path.resolve(__dirname, '../../test')).toString() }];
        await assertCompletions('html { background-image: url(./|)', {
            items: [
                { label: 'about.html', resultText: 'html { background-image: url(./about.html)' }
            ]
        }, testUri, folders);
        await assertCompletions('html { background-image: url(./a|)', {
            items: [
                { label: 'about.html', resultText: 'html { background-image: url(./about.html)' }
            ]
        }, testUri, folders);
        await assertCompletions('html { background-image: url(../|src/)', {
            items: [
                { label: 'about/', resultText: 'html { background-image: url(../about/)' }
            ]
        }, testUri, folders);
        await assertCompletions('html { background-image: url(../s|rc/)', {
            items: [
                { label: 'about/', resultText: 'html { background-image: url(../about/)' }
            ]
        }, testUri, folders);
    });
    test('CSS @import Path completion', async function () {
        let testUri = vscode_uri_1.URI.file(path.resolve(__dirname, '../../test/pathCompletionFixtures/about/about.css')).toString();
        let folders = [{ name: 'x', uri: vscode_uri_1.URI.file(path.resolve(__dirname, '../../test')).toString() }];
        await assertCompletions(`@import './|'`, {
            items: [
                { label: 'about.html', resultText: `@import './about.html'` },
            ]
        }, testUri, folders);
        await assertCompletions(`@import '../|'`, {
            items: [
                { label: 'about/', resultText: `@import '../about/'` },
                { label: 'scss/', resultText: `@import '../scss/'` },
                { label: 'index.html', resultText: `@import '../index.html'` },
                { label: 'src/', resultText: `@import '../src/'` }
            ]
        }, testUri, folders);
    });
    /**
     * For SCSS, `@import 'foo';` can be used for importing partial file `_foo.scss`
     */
    test('SCSS @import Path completion', async function () {
        let testCSSUri = vscode_uri_1.URI.file(path.resolve(__dirname, '../../test/pathCompletionFixtures/about/about.css')).toString();
        let folders = [{ name: 'x', uri: vscode_uri_1.URI.file(path.resolve(__dirname, '../../test')).toString() }];
        /**
         * We are in a CSS file, so no special treatment for SCSS partial files
        */
        await assertCompletions(`@import '../scss/|'`, {
            items: [
                { label: 'main.scss', resultText: `@import '../scss/main.scss'` },
                { label: '_foo.scss', resultText: `@import '../scss/_foo.scss'` }
            ]
        }, testCSSUri, folders);
        let testSCSSUri = vscode_uri_1.URI.file(path.resolve(__dirname, '../../test/pathCompletionFixtures/scss/main.scss')).toString();
        await assertCompletions(`@import './|'`, {
            items: [
                { label: '_foo.scss', resultText: `@import './foo'` }
            ]
        }, testSCSSUri, folders, 'scss');
    });
    test('Completion should ignore files/folders starting with dot', async function () {
        let testUri = vscode_uri_1.URI.file(path.resolve(__dirname, '../../test/pathCompletionFixtures/about/about.css')).toString();
        let folders = [{ name: 'x', uri: vscode_uri_1.URI.file(path.resolve(__dirname, '../../test')).toString() }];
        await assertCompletions('html { background-image: url("../|")', {
            count: 4
        }, testUri, folders);
    });
});
//# sourceMappingURL=completion.test.js.map