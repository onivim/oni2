"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.removeTag = void 0;
const vscode = require("vscode");
const util_1 = require("./util");
function removeTag() {
    if (!util_1.validate(false) || !vscode.window.activeTextEditor) {
        return;
    }
    const editor = vscode.window.activeTextEditor;
    let rootNode = util_1.parseDocument(editor.document);
    if (!rootNode) {
        return;
    }
    let indentInSpaces = '';
    const tabSize = editor.options.tabSize ? +editor.options.tabSize : 0;
    for (let i = 0; i < tabSize || 0; i++) {
        indentInSpaces += ' ';
    }
    let rangesToRemove = [];
    editor.selections.reverse().forEach(selection => {
        rangesToRemove = rangesToRemove.concat(getRangeToRemove(editor, rootNode, selection, indentInSpaces));
    });
    return editor.edit(editBuilder => {
        rangesToRemove.forEach(range => {
            editBuilder.replace(range, '');
        });
    });
}
exports.removeTag = removeTag;
function getRangeToRemove(editor, rootNode, selection, indentInSpaces) {
    let nodeToUpdate = util_1.getHtmlNode(editor.document, rootNode, selection.start, true);
    if (!nodeToUpdate) {
        return [];
    }
    let openRange = new vscode.Range(nodeToUpdate.open.start, nodeToUpdate.open.end);
    let closeRange = null;
    if (nodeToUpdate.close) {
        closeRange = new vscode.Range(nodeToUpdate.close.start, nodeToUpdate.close.end);
    }
    let ranges = [openRange];
    if (closeRange) {
        for (let i = openRange.start.line + 1; i <= closeRange.start.line; i++) {
            let lineContent = editor.document.lineAt(i).text;
            if (lineContent.startsWith('\t')) {
                ranges.push(new vscode.Range(i, 0, i, 1));
            }
            else if (lineContent.startsWith(indentInSpaces)) {
                ranges.push(new vscode.Range(i, 0, i, indentInSpaces.length));
            }
        }
        ranges.push(closeRange);
    }
    return ranges;
}
//# sourceMappingURL=removeTag.js.map