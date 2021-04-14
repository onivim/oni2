"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const assert = require("assert");
require("mocha");
const vscode_1 = require("vscode");
const defaultCompletionProvider_1 = require("../defaultCompletionProvider");
const testUtils_1 = require("./testUtils");
const completionProvider = new defaultCompletionProvider_1.DefaultCompletionItemProvider();
suite('Tests for completion in CSS embedded in HTML', () => {
    teardown(() => {
        // close all editors
        return testUtils_1.closeAllEditors;
    });
    test('style attribute & attribute value in html', async () => {
        await testHtmlCompletionProvider('<div style="|"', [{ label: 'padding: ;' }]);
        await testHtmlCompletionProvider(`<div style='|'`, [{ label: 'padding: ;' }]);
        await testHtmlCompletionProvider(`<div style='p|'`, [{ label: 'padding: ;' }]);
        await testHtmlCompletionProvider(`<div style='color: #0|'`, [{ label: '#000000' }]);
    });
    // https://github.com/microsoft/vscode/issues/79766
    test('#79766, correct region determination', async () => {
        await testHtmlCompletionProvider(`<div style="color: #000">di|</div>`, [
            { label: 'div', documentation: `<div>|</div>` }
        ]);
    });
    // https://github.com/microsoft/vscode/issues/86941
    test('#86941, widows should not be completed', async () => {
        await testCssCompletionProvider(`.foo { wi| }`, [
            { label: 'widows: ;', documentation: `widows: ;` }
        ]);
    });
});
function testHtmlCompletionProvider(contents, expectedItems) {
    const cursorPos = contents.indexOf('|');
    const htmlContents = contents.slice(0, cursorPos) + contents.slice(cursorPos + 1);
    return testUtils_1.withRandomFileEditor(htmlContents, 'html', async (editor, _doc) => {
        const selection = new vscode_1.Selection(editor.document.positionAt(cursorPos), editor.document.positionAt(cursorPos));
        editor.selection = selection;
        const cancelSrc = new vscode_1.CancellationTokenSource();
        const completionPromise = completionProvider.provideCompletionItems(editor.document, editor.selection.active, cancelSrc.token, { triggerKind: vscode_1.CompletionTriggerKind.Invoke });
        if (!completionPromise) {
            return Promise.resolve();
        }
        const completionList = await completionPromise;
        if (!completionList || !completionList.items || !completionList.items.length) {
            return Promise.resolve();
        }
        expectedItems.forEach(eItem => {
            const matches = completionList.items.filter(i => i.label === eItem.label);
            const match = matches && matches.length > 0 ? matches[0] : undefined;
            assert.ok(match, `Didn't find completion item with label ${eItem.label}`);
            if (match) {
                assert.equal(match.detail, 'Emmet Abbreviation', `Match needs to come from Emmet`);
                if (eItem.documentation) {
                    assert.equal(match.documentation, eItem.documentation, `Emmet completion Documentation doesn't match`);
                }
            }
        });
        return Promise.resolve();
    });
}
function testCssCompletionProvider(contents, expectedItems) {
    const cursorPos = contents.indexOf('|');
    const cssContents = contents.slice(0, cursorPos) + contents.slice(cursorPos + 1);
    return testUtils_1.withRandomFileEditor(cssContents, 'css', async (editor, _doc) => {
        const selection = new vscode_1.Selection(editor.document.positionAt(cursorPos), editor.document.positionAt(cursorPos));
        editor.selection = selection;
        const cancelSrc = new vscode_1.CancellationTokenSource();
        const completionPromise = completionProvider.provideCompletionItems(editor.document, editor.selection.active, cancelSrc.token, { triggerKind: vscode_1.CompletionTriggerKind.Invoke });
        if (!completionPromise) {
            return Promise.resolve();
        }
        const completionList = await completionPromise;
        if (!completionList || !completionList.items || !completionList.items.length) {
            return Promise.resolve();
        }
        expectedItems.forEach(eItem => {
            const matches = completionList.items.filter(i => i.label === eItem.label);
            const match = matches && matches.length > 0 ? matches[0] : undefined;
            assert.ok(match, `Didn't find completion item with label ${eItem.label}`);
            if (match) {
                assert.equal(match.detail, 'Emmet Abbreviation', `Match needs to come from Emmet`);
                if (eItem.documentation) {
                    assert.equal(match.documentation, eItem.documentation, `Emmet completion Documentation doesn't match`);
                }
            }
        });
        return Promise.resolve();
    });
}
//# sourceMappingURL=completion.test.js.map