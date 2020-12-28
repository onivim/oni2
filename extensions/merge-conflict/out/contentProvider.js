"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const vscode = require("vscode");
class MergeConflictContentProvider {
    constructor(context) {
        this.context = context;
    }
    begin() {
        this.context.subscriptions.push(vscode.workspace.registerTextDocumentContentProvider(MergeConflictContentProvider.scheme, this));
    }
    dispose() {
    }
    async provideTextDocumentContent(uri) {
        try {
            const { scheme, ranges } = JSON.parse(uri.query);
            // complete diff
            const document = await vscode.workspace.openTextDocument(uri.with({ scheme, query: '' }));
            let text = '';
            let lastPosition = new vscode.Position(0, 0);
            ranges.forEach(rangeObj => {
                let [conflictRange, fullRange] = rangeObj;
                const [start, end] = conflictRange;
                const [fullStart, fullEnd] = fullRange;
                text += document.getText(new vscode.Range(lastPosition.line, lastPosition.character, fullStart.line, fullStart.character));
                text += document.getText(new vscode.Range(start.line, start.character, end.line, end.character));
                lastPosition = new vscode.Position(fullEnd.line, fullEnd.character);
            });
            let documentEnd = document.lineAt(document.lineCount - 1).range.end;
            text += document.getText(new vscode.Range(lastPosition.line, lastPosition.character, documentEnd.line, documentEnd.character));
            return text;
        }
        catch (ex) {
            await vscode.window.showErrorMessage('Unable to show comparison');
            return null;
        }
    }
}
exports.default = MergeConflictContentProvider;
MergeConflictContentProvider.scheme = 'merge-conflict.conflict-diff';
//# sourceMappingURL=contentProvider.js.map