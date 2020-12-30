"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.getCSSMode = void 0;
const languageModelCache_1 = require("../languageModelCache");
const languageModes_1 = require("./languageModes");
const embeddedSupport_1 = require("./embeddedSupport");
function getCSSMode(cssLanguageService, documentRegions, workspace) {
    let embeddedCSSDocuments = languageModelCache_1.getLanguageModelCache(10, 60, document => documentRegions.get(document).getEmbeddedDocument('css'));
    let cssStylesheets = languageModelCache_1.getLanguageModelCache(10, 60, document => cssLanguageService.parseStylesheet(document));
    return {
        getId() {
            return 'css';
        },
        async doValidation(document, settings = workspace.settings) {
            let embedded = embeddedCSSDocuments.get(document);
            return cssLanguageService.doValidation(embedded, cssStylesheets.get(embedded), settings && settings.css);
        },
        async doComplete(document, position, documentContext, _settings = workspace.settings) {
            let embedded = embeddedCSSDocuments.get(document);
            const stylesheet = cssStylesheets.get(embedded);
            return cssLanguageService.doComplete2(embedded, position, stylesheet, documentContext) || languageModes_1.CompletionList.create();
        },
        async doHover(document, position, settings) {
            var _a;
            let embedded = embeddedCSSDocuments.get(document);
            return cssLanguageService.doHover(embedded, position, cssStylesheets.get(embedded), (_a = settings === null || settings === void 0 ? void 0 : settings.html) === null || _a === void 0 ? void 0 : _a.hover);
        },
        async findDocumentHighlight(document, position) {
            let embedded = embeddedCSSDocuments.get(document);
            return cssLanguageService.findDocumentHighlights(embedded, position, cssStylesheets.get(embedded));
        },
        async findDocumentSymbols(document) {
            let embedded = embeddedCSSDocuments.get(document);
            return cssLanguageService.findDocumentSymbols(embedded, cssStylesheets.get(embedded)).filter(s => s.name !== embeddedSupport_1.CSS_STYLE_RULE);
        },
        async findDefinition(document, position) {
            let embedded = embeddedCSSDocuments.get(document);
            return cssLanguageService.findDefinition(embedded, position, cssStylesheets.get(embedded));
        },
        async findReferences(document, position) {
            let embedded = embeddedCSSDocuments.get(document);
            return cssLanguageService.findReferences(embedded, position, cssStylesheets.get(embedded));
        },
        async findDocumentColors(document) {
            let embedded = embeddedCSSDocuments.get(document);
            return cssLanguageService.findDocumentColors(embedded, cssStylesheets.get(embedded));
        },
        async getColorPresentations(document, color, range) {
            let embedded = embeddedCSSDocuments.get(document);
            return cssLanguageService.getColorPresentations(embedded, cssStylesheets.get(embedded), color, range);
        },
        async getFoldingRanges(document) {
            let embedded = embeddedCSSDocuments.get(document);
            return cssLanguageService.getFoldingRanges(embedded, {});
        },
        async getSelectionRange(document, position) {
            let embedded = embeddedCSSDocuments.get(document);
            return cssLanguageService.getSelectionRanges(embedded, [position], cssStylesheets.get(embedded))[0];
        },
        onDocumentRemoved(document) {
            embeddedCSSDocuments.onDocumentRemoved(document);
            cssStylesheets.onDocumentRemoved(document);
        },
        dispose() {
            embeddedCSSDocuments.dispose();
            cssStylesheets.dispose();
        }
    };
}
exports.getCSSMode = getCSSMode;
//# sourceMappingURL=cssMode.js.map