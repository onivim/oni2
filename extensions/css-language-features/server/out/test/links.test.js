"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
require("mocha");
const assert = require("assert");
const vscode_uri_1 = require("vscode-uri");
const path_1 = require("path");
const vscode_languageserver_types_1 = require("vscode-languageserver-types");
const vscode_css_languageservice_1 = require("vscode-css-languageservice");
const documentContext_1 = require("../utils/documentContext");
suite('Links', () => {
    const cssLanguageService = vscode_css_languageservice_1.getCSSLanguageService();
    let assertLink = function (links, expected, document) {
        let matches = links.filter(link => {
            return document.offsetAt(link.range.start) === expected.offset;
        });
        assert.equal(matches.length, 1, `${expected.offset} should only existing once: Actual: ${links.map(l => document.offsetAt(l.range.start)).join(', ')}`);
        let match = matches[0];
        assert.equal(document.getText(match.range), expected.value);
        assert.equal(match.target, expected.target);
    };
    function assertLinks(value, expected, testUri, workspaceFolders, lang = 'css') {
        const offset = value.indexOf('|');
        value = value.substr(0, offset) + value.substr(offset + 1);
        const document = vscode_languageserver_types_1.TextDocument.create(testUri, lang, 0, value);
        if (!workspaceFolders) {
            workspaceFolders = [{ name: 'x', uri: testUri.substr(0, testUri.lastIndexOf('/')) }];
        }
        const context = documentContext_1.getDocumentContext(testUri, workspaceFolders);
        const stylesheet = cssLanguageService.parseStylesheet(document);
        let links = cssLanguageService.findDocumentLinks(document, stylesheet, context);
        assert.equal(links.length, expected.length);
        for (let item of expected) {
            assertLink(links, item, document);
        }
    }
    function getTestResource(path) {
        return vscode_uri_1.URI.file(path_1.resolve(__dirname, '../../test/linksTestFixtures', path)).toString();
    }
    test('url links', function () {
        let testUri = getTestResource('about.css');
        let folders = [{ name: 'x', uri: getTestResource('') }];
        assertLinks('html { background-image: url("hello.html|")', [{ offset: 29, value: '"hello.html"', target: getTestResource('hello.html') }], testUri, folders);
    });
    test('node module resolving', function () {
        let testUri = getTestResource('about.css');
        let folders = [{ name: 'x', uri: getTestResource('') }];
        assertLinks('html { background-image: url("~foo/hello.html|")', [{ offset: 29, value: '"~foo/hello.html"', target: getTestResource('node_modules/foo/hello.html') }], testUri, folders);
    });
});
//# sourceMappingURL=links.test.js.map