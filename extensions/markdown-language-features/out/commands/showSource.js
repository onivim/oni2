"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.ShowSourceCommand = void 0;
const vscode = require("vscode");
class ShowSourceCommand {
    constructor(previewManager) {
        this.previewManager = previewManager;
        this.id = 'markdown.showSource';
    }
    execute() {
        const { activePreviewResource, activePreviewResourceColumn } = this.previewManager;
        if (activePreviewResource && activePreviewResourceColumn) {
            return vscode.workspace.openTextDocument(activePreviewResource).then(document => {
                vscode.window.showTextDocument(document, activePreviewResourceColumn);
            });
        }
        return undefined;
    }
}
exports.ShowSourceCommand = ShowSourceCommand;
//# sourceMappingURL=showSource.js.map