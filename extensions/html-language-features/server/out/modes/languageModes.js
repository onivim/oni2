"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __exportStar = (this && this.__exportStar) || function(m, exports) {
    for (var p in m) if (p !== "default" && !Object.prototype.hasOwnProperty.call(exports, p)) __createBinding(exports, m, p);
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.getLanguageModes = void 0;
const vscode_css_languageservice_1 = require("vscode-css-languageservice");
const vscode_html_languageservice_1 = require("vscode-html-languageservice");
const languageModelCache_1 = require("../languageModelCache");
const cssMode_1 = require("./cssMode");
const embeddedSupport_1 = require("./embeddedSupport");
const htmlMode_1 = require("./htmlMode");
const javascriptMode_1 = require("./javascriptMode");
__exportStar(require("vscode-html-languageservice"), exports);
function getLanguageModes(supportedLanguages, workspace, clientCapabilities, requestService) {
    const htmlLanguageService = vscode_html_languageservice_1.getLanguageService({ clientCapabilities, fileSystemProvider: requestService });
    const cssLanguageService = vscode_css_languageservice_1.getCSSLanguageService({ clientCapabilities, fileSystemProvider: requestService });
    let documentRegions = languageModelCache_1.getLanguageModelCache(10, 60, document => embeddedSupport_1.getDocumentRegions(htmlLanguageService, document));
    let modelCaches = [];
    modelCaches.push(documentRegions);
    let modes = Object.create(null);
    modes['html'] = htmlMode_1.getHTMLMode(htmlLanguageService, workspace);
    if (supportedLanguages['css']) {
        modes['css'] = cssMode_1.getCSSMode(cssLanguageService, documentRegions, workspace);
    }
    if (supportedLanguages['javascript']) {
        modes['javascript'] = javascriptMode_1.getJavaScriptMode(documentRegions, 'javascript', workspace);
        modes['typescript'] = javascriptMode_1.getJavaScriptMode(documentRegions, 'typescript', workspace);
    }
    return {
        async updateDataProviders(dataProviders) {
            htmlLanguageService.setDataProviders(true, dataProviders);
        },
        getModeAtPosition(document, position) {
            let languageId = documentRegions.get(document).getLanguageAtPosition(position);
            if (languageId) {
                return modes[languageId];
            }
            return undefined;
        },
        getModesInRange(document, range) {
            return documentRegions.get(document).getLanguageRanges(range).map(r => {
                return {
                    start: r.start,
                    end: r.end,
                    mode: r.languageId && modes[r.languageId],
                    attributeValue: r.attributeValue
                };
            });
        },
        getAllModesInDocument(document) {
            let result = [];
            for (let languageId of documentRegions.get(document).getLanguagesInDocument()) {
                let mode = modes[languageId];
                if (mode) {
                    result.push(mode);
                }
            }
            return result;
        },
        getAllModes() {
            let result = [];
            for (let languageId in modes) {
                let mode = modes[languageId];
                if (mode) {
                    result.push(mode);
                }
            }
            return result;
        },
        getMode(languageId) {
            return modes[languageId];
        },
        onDocumentRemoved(document) {
            modelCaches.forEach(mc => mc.onDocumentRemoved(document));
            for (let mode in modes) {
                modes[mode].onDocumentRemoved(document);
            }
        },
        dispose() {
            modelCaches.forEach(mc => mc.dispose());
            modelCaches = [];
            for (let mode in modes) {
                modes[mode].dispose();
            }
            modes = {};
        }
    };
}
exports.getLanguageModes = getLanguageModes;
//# sourceMappingURL=languageModes.js.map