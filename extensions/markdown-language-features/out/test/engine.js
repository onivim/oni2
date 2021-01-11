"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.createNewMarkdownEngine = void 0;
const vscode = require("vscode");
const markdownEngine_1 = require("../markdownEngine");
const markdownExtensions_1 = require("../markdownExtensions");
const slugify_1 = require("../slugify");
const dispose_1 = require("../util/dispose");
const emptyContributions = new class extends dispose_1.Disposable {
    constructor() {
        super(...arguments);
        this.extensionUri = vscode.Uri.file('/');
        this.contributions = markdownExtensions_1.MarkdownContributions.Empty;
        this.onContributionsChanged = this._register(new vscode.EventEmitter()).event;
    }
};
function createNewMarkdownEngine() {
    return new markdownEngine_1.MarkdownEngine(emptyContributions, slugify_1.githubSlugifier);
}
exports.createNewMarkdownEngine = createNewMarkdownEngine;
//# sourceMappingURL=engine.js.map