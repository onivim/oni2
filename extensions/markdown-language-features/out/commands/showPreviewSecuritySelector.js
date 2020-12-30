"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.ShowPreviewSecuritySelectorCommand = void 0;
const vscode = require("vscode");
const file_1 = require("../util/file");
class ShowPreviewSecuritySelectorCommand {
    constructor(previewSecuritySelector, previewManager) {
        this.previewSecuritySelector = previewSecuritySelector;
        this.previewManager = previewManager;
        this.id = 'markdown.showPreviewSecuritySelector';
    }
    execute(resource) {
        if (this.previewManager.activePreviewResource) {
            this.previewSecuritySelector.showSecuritySelectorForResource(this.previewManager.activePreviewResource);
        }
        else if (resource) {
            const source = vscode.Uri.parse(resource);
            this.previewSecuritySelector.showSecuritySelectorForResource(source.query ? vscode.Uri.parse(source.query) : source);
        }
        else if (vscode.window.activeTextEditor && file_1.isMarkdownFile(vscode.window.activeTextEditor.document)) {
            this.previewSecuritySelector.showSecuritySelectorForResource(vscode.window.activeTextEditor.document.uri);
        }
    }
}
exports.ShowPreviewSecuritySelectorCommand = ShowPreviewSecuritySelectorCommand;
//# sourceMappingURL=showPreviewSecuritySelector.js.map