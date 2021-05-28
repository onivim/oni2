"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.splitJoinTag = void 0;
const vscode = require("vscode");
const util_1 = require("./util");
function splitJoinTag() {
    if (!util_1.validate(false) || !vscode.window.activeTextEditor) {
        return;
    }
    const editor = vscode.window.activeTextEditor;
    let rootNode = util_1.parseDocument(editor.document);
    if (!rootNode) {
        return;
    }
    return editor.edit(editBuilder => {
        editor.selections.reverse().forEach(selection => {
            let nodeToUpdate = util_1.getHtmlNode(editor.document, rootNode, selection.start, true);
            if (nodeToUpdate) {
                let textEdit = getRangesToReplace(editor.document, nodeToUpdate);
                editBuilder.replace(textEdit.range, textEdit.newText);
            }
        });
    });
}
exports.splitJoinTag = splitJoinTag;
function getRangesToReplace(document, nodeToUpdate) {
    let rangeToReplace;
    let textToReplaceWith;
    if (!nodeToUpdate.close) {
        // Split Tag
        let nodeText = document.getText(new vscode.Range(nodeToUpdate.start, nodeToUpdate.end));
        let m = nodeText.match(/(\s*\/)?>$/);
        let end = nodeToUpdate.end;
        let start = m ? end.translate(0, -m[0].length) : end;
        rangeToReplace = new vscode.Range(start, end);
        textToReplaceWith = `></${nodeToUpdate.name}>`;
    }
    else {
        // Join Tag
        let start = nodeToUpdate.open.end.translate(0, -1);
        let end = nodeToUpdate.end;
        rangeToReplace = new vscode.Range(start, end);
        textToReplaceWith = '/>';
        const emmetMode = util_1.getEmmetMode(document.languageId, []) || '';
        const emmetConfig = util_1.getEmmetConfiguration(emmetMode);
        if (emmetMode && emmetConfig.syntaxProfiles[emmetMode] &&
            (emmetConfig.syntaxProfiles[emmetMode]['selfClosingStyle'] === 'xhtml' || emmetConfig.syntaxProfiles[emmetMode]['self_closing_tag'] === 'xhtml')) {
            textToReplaceWith = ' ' + textToReplaceWith;
        }
    }
    return new vscode.TextEdit(rangeToReplace, textToReplaceWith);
}
//# sourceMappingURL=splitJoinTag.js.map