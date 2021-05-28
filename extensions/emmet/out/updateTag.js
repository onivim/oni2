"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.updateTag = void 0;
const vscode = require("vscode");
const util_1 = require("./util");
function updateTag(tagName) {
    if (!util_1.validate(false) || !vscode.window.activeTextEditor) {
        return;
    }
    let editor = vscode.window.activeTextEditor;
    let rootNode = util_1.parseDocument(editor.document);
    if (!rootNode) {
        return;
    }
    let rangesToUpdate = [];
    editor.selections.reverse().forEach(selection => {
        rangesToUpdate = rangesToUpdate.concat(getRangesToUpdate(editor, selection, rootNode));
    });
    return editor.edit(editBuilder => {
        rangesToUpdate.forEach(range => {
            editBuilder.replace(range, tagName);
        });
    });
}
exports.updateTag = updateTag;
function getRangesToUpdate(editor, selection, rootNode) {
    let nodeToUpdate = util_1.getHtmlNode(editor.document, rootNode, selection.start, true);
    if (!nodeToUpdate) {
        return [];
    }
    let openStart = nodeToUpdate.open.start.translate(0, 1);
    let openEnd = openStart.translate(0, nodeToUpdate.name.length);
    let ranges = [new vscode.Range(openStart, openEnd)];
    if (nodeToUpdate.close) {
        let closeStart = nodeToUpdate.close.start.translate(0, 2);
        let closeEnd = nodeToUpdate.close.end.translate(0, -1);
        ranges.push(new vscode.Range(closeStart, closeEnd));
    }
    return ranges;
}
//# sourceMappingURL=updateTag.js.map