"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.getHTMLMode = void 0;
const languageModelCache_1 = require("../languageModelCache");
function getHTMLMode(htmlLanguageService, workspace) {
    let htmlDocuments = languageModelCache_1.getLanguageModelCache(10, 60, document => htmlLanguageService.parseHTMLDocument(document));
    return {
        getId() {
            return 'html';
        },
        async getSelectionRange(document, position) {
            return htmlLanguageService.getSelectionRanges(document, [position])[0];
        },
        doComplete(document, position, documentContext, settings = workspace.settings) {
            let options = settings && settings.html && settings.html.suggest;
            let doAutoComplete = settings && settings.html && settings.html.autoClosingTags;
            if (doAutoComplete) {
                options.hideAutoCompleteProposals = true;
            }
            const htmlDocument = htmlDocuments.get(document);
            let completionList = htmlLanguageService.doComplete2(document, position, htmlDocument, documentContext, options);
            return completionList;
        },
        async doHover(document, position, settings) {
            var _a;
            return htmlLanguageService.doHover(document, position, htmlDocuments.get(document), (_a = settings === null || settings === void 0 ? void 0 : settings.html) === null || _a === void 0 ? void 0 : _a.hover);
        },
        async findDocumentHighlight(document, position) {
            return htmlLanguageService.findDocumentHighlights(document, position, htmlDocuments.get(document));
        },
        async findDocumentLinks(document, documentContext) {
            return htmlLanguageService.findDocumentLinks(document, documentContext);
        },
        async findDocumentSymbols(document) {
            return htmlLanguageService.findDocumentSymbols(document, htmlDocuments.get(document));
        },
        async format(document, range, formatParams, settings = workspace.settings) {
            let formatSettings = settings && settings.html && settings.html.format;
            if (formatSettings) {
                formatSettings = merge(formatSettings, {});
            }
            else {
                formatSettings = {};
            }
            if (formatSettings.contentUnformatted) {
                formatSettings.contentUnformatted = formatSettings.contentUnformatted + ',script';
            }
            else {
                formatSettings.contentUnformatted = 'script';
            }
            formatSettings = merge(formatParams, formatSettings);
            return htmlLanguageService.format(document, range, formatSettings);
        },
        async getFoldingRanges(document) {
            return htmlLanguageService.getFoldingRanges(document);
        },
        async doAutoClose(document, position) {
            let offset = document.offsetAt(position);
            let text = document.getText();
            if (offset > 0 && text.charAt(offset - 1).match(/[>\/]/g)) {
                return htmlLanguageService.doTagComplete(document, position, htmlDocuments.get(document));
            }
            return null;
        },
        async doRename(document, position, newName) {
            const htmlDocument = htmlDocuments.get(document);
            return htmlLanguageService.doRename(document, position, newName, htmlDocument);
        },
        async onDocumentRemoved(document) {
            htmlDocuments.onDocumentRemoved(document);
        },
        async findMatchingTagPosition(document, position) {
            const htmlDocument = htmlDocuments.get(document);
            return htmlLanguageService.findMatchingTagPosition(document, position, htmlDocument);
        },
        async doLinkedEditing(document, position) {
            const htmlDocument = htmlDocuments.get(document);
            return htmlLanguageService.findLinkedEditingRanges(document, position, htmlDocument);
        },
        dispose() {
            htmlDocuments.dispose();
        }
    };
}
exports.getHTMLMode = getHTMLMode;
function merge(src, dst) {
    for (const key in src) {
        if (src.hasOwnProperty(key)) {
            dst[key] = src[key];
        }
    }
    return dst;
}
//# sourceMappingURL=htmlMode.js.map