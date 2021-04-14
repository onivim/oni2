"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.fetchSelectItem = void 0;
const vscode = require("vscode");
const util_1 = require("./util");
const selectItemHTML_1 = require("./selectItemHTML");
const selectItemStylesheet_1 = require("./selectItemStylesheet");
function fetchSelectItem(direction) {
    if (!util_1.validate() || !vscode.window.activeTextEditor) {
        return;
    }
    const editor = vscode.window.activeTextEditor;
    let rootNode = util_1.parseDocument(editor.document);
    if (!rootNode) {
        return;
    }
    let newSelections = [];
    editor.selections.forEach(selection => {
        const selectionStart = selection.isReversed ? selection.active : selection.anchor;
        const selectionEnd = selection.isReversed ? selection.anchor : selection.active;
        let updatedSelection;
        if (util_1.isStyleSheet(editor.document.languageId)) {
            updatedSelection = direction === 'next' ? selectItemStylesheet_1.nextItemStylesheet(selectionStart, selectionEnd, rootNode) : selectItemStylesheet_1.prevItemStylesheet(selectionStart, selectionEnd, rootNode);
        }
        else {
            updatedSelection = direction === 'next' ? selectItemHTML_1.nextItemHTML(selectionStart, selectionEnd, editor, rootNode) : selectItemHTML_1.prevItemHTML(selectionStart, selectionEnd, editor, rootNode);
        }
        newSelections.push(updatedSelection ? updatedSelection : selection);
    });
    editor.selections = newSelections;
    editor.revealRange(editor.selections[editor.selections.length - 1]);
}
exports.fetchSelectItem = fetchSelectItem;
//# sourceMappingURL=selectItem.js.map