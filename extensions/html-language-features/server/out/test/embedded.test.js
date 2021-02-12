"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
require("mocha");
const assert = require("assert");
const embeddedSupport = require("../modes/embeddedSupport");
const vscode_html_languageservice_1 = require("vscode-html-languageservice");
const languageModes_1 = require("../modes/languageModes");
suite('HTML Embedded Support', () => {
    const htmlLanguageService = vscode_html_languageservice_1.getLanguageService();
    function assertLanguageId(value, expectedLanguageId) {
        const offset = value.indexOf('|');
        value = value.substr(0, offset) + value.substr(offset + 1);
        const document = languageModes_1.TextDocument.create('test://test/test.html', 'html', 0, value);
        const position = document.positionAt(offset);
        const docRegions = embeddedSupport.getDocumentRegions(htmlLanguageService, document);
        const languageId = docRegions.getLanguageAtPosition(position);
        assert.equal(languageId, expectedLanguageId);
    }
    function assertEmbeddedLanguageContent(value, languageId, expectedContent) {
        const document = languageModes_1.TextDocument.create('test://test/test.html', 'html', 0, value);
        const docRegions = embeddedSupport.getDocumentRegions(htmlLanguageService, document);
        const content = docRegions.getEmbeddedDocument(languageId);
        assert.equal(content.getText(), expectedContent);
    }
    test('Styles', function () {
        assertLanguageId('|<html><style>foo { }</style></html>', 'html');
        assertLanguageId('<html|><style>foo { }</style></html>', 'html');
        assertLanguageId('<html><st|yle>foo { }</style></html>', 'html');
        assertLanguageId('<html><style>|foo { }</style></html>', 'css');
        assertLanguageId('<html><style>foo| { }</style></html>', 'css');
        assertLanguageId('<html><style>foo { }|</style></html>', 'css');
        assertLanguageId('<html><style>foo { }</sty|le></html>', 'html');
    });
    test('Styles - Incomplete HTML', function () {
        assertLanguageId('|<html><style>foo { }', 'html');
        assertLanguageId('<html><style>fo|o { }', 'css');
        assertLanguageId('<html><style>foo { }|', 'css');
    });
    test('Style in attribute', function () {
        assertLanguageId('<div id="xy" |style="color: red"/>', 'html');
        assertLanguageId('<div id="xy" styl|e="color: red"/>', 'html');
        assertLanguageId('<div id="xy" style=|"color: red"/>', 'html');
        assertLanguageId('<div id="xy" style="|color: red"/>', 'css');
        assertLanguageId('<div id="xy" style="color|: red"/>', 'css');
        assertLanguageId('<div id="xy" style="color: red|"/>', 'css');
        assertLanguageId('<div id="xy" style="color: red"|/>', 'html');
        assertLanguageId('<div id="xy" style=\'color: r|ed\'/>', 'css');
        assertLanguageId('<div id="xy" style|=color:red/>', 'html');
        assertLanguageId('<div id="xy" style=|color:red/>', 'css');
        assertLanguageId('<div id="xy" style=color:r|ed/>', 'css');
        assertLanguageId('<div id="xy" style=color:red|/>', 'css');
        assertLanguageId('<div id="xy" style=color:red/|>', 'html');
    });
    test('Style content', function () {
        assertEmbeddedLanguageContent('<html><style>foo { }</style></html>', 'css', '             foo { }               ');
        assertEmbeddedLanguageContent('<html><script>var i = 0;</script></html>', 'css', '                                        ');
        assertEmbeddedLanguageContent('<html><style>foo { }</style>Hello<style>foo { }</style></html>', 'css', '             foo { }                    foo { }               ');
        assertEmbeddedLanguageContent('<html>\n  <style>\n    foo { }  \n  </style>\n</html>\n', 'css', '\n         \n    foo { }  \n  \n\n');
        assertEmbeddedLanguageContent('<div style="color: red"></div>', 'css', '         __{color: red}       ');
        assertEmbeddedLanguageContent('<div style=color:red></div>', 'css', '        __{color:red}      ');
    });
    test('Scripts', function () {
        assertLanguageId('|<html><script>var i = 0;</script></html>', 'html');
        assertLanguageId('<html|><script>var i = 0;</script></html>', 'html');
        assertLanguageId('<html><scr|ipt>var i = 0;</script></html>', 'html');
        assertLanguageId('<html><script>|var i = 0;</script></html>', 'javascript');
        assertLanguageId('<html><script>var| i = 0;</script></html>', 'javascript');
        assertLanguageId('<html><script>var i = 0;|</script></html>', 'javascript');
        assertLanguageId('<html><script>var i = 0;</scr|ipt></html>', 'html');
        assertLanguageId('<script type="text/javascript">var| i = 0;</script>', 'javascript');
        assertLanguageId('<script type="text/ecmascript">var| i = 0;</script>', 'javascript');
        assertLanguageId('<script type="application/javascript">var| i = 0;</script>', 'javascript');
        assertLanguageId('<script type="application/ecmascript">var| i = 0;</script>', 'javascript');
        assertLanguageId('<script type="application/typescript">var| i = 0;</script>', undefined);
        assertLanguageId('<script type=\'text/javascript\'>var| i = 0;</script>', 'javascript');
    });
    test('Scripts in attribute', function () {
        assertLanguageId('<div |onKeyUp="foo()" onkeydown=\'bar()\'/>', 'html');
        assertLanguageId('<div onKeyUp=|"foo()" onkeydown=\'bar()\'/>', 'html');
        assertLanguageId('<div onKeyUp="|foo()" onkeydown=\'bar()\'/>', 'javascript');
        assertLanguageId('<div onKeyUp="foo(|)" onkeydown=\'bar()\'/>', 'javascript');
        assertLanguageId('<div onKeyUp="foo()|" onkeydown=\'bar()\'/>', 'javascript');
        assertLanguageId('<div onKeyUp="foo()"| onkeydown=\'bar()\'/>', 'html');
        assertLanguageId('<div onKeyUp="foo()" onkeydown=|\'bar()\'/>', 'html');
        assertLanguageId('<div onKeyUp="foo()" onkeydown=\'|bar()\'/>', 'javascript');
        assertLanguageId('<div onKeyUp="foo()" onkeydown=\'bar()|\'/>', 'javascript');
        assertLanguageId('<div onKeyUp="foo()" onkeydown=\'bar()\'|/>', 'html');
        assertLanguageId('<DIV ONKEYUP|=foo()</DIV>', 'html');
        assertLanguageId('<DIV ONKEYUP=|foo()</DIV>', 'javascript');
        assertLanguageId('<DIV ONKEYUP=f|oo()</DIV>', 'javascript');
        assertLanguageId('<DIV ONKEYUP=foo(|)</DIV>', 'javascript');
        assertLanguageId('<DIV ONKEYUP=foo()|</DIV>', 'javascript');
        assertLanguageId('<DIV ONKEYUP=foo()<|/DIV>', 'html');
        assertLanguageId('<label data-content="|Checkbox"/>', 'html');
        assertLanguageId('<label on="|Checkbox"/>', 'html');
    });
    test('Script content', function () {
        assertEmbeddedLanguageContent('<html><script>var i = 0;</script></html>', 'javascript', '              var i = 0;                ');
        assertEmbeddedLanguageContent('<script type="text/javascript">var i = 0;</script>', 'javascript', '                               var i = 0;         ');
        assertEmbeddedLanguageContent('<div onKeyUp="foo()" onkeydown="bar()"/>', 'javascript', '              foo();            bar();  ');
    });
});
//# sourceMappingURL=embedded.test.js.map