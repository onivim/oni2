"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const vscode_1 = require("vscode");
const markedTextUtil_1 = require("./utils/markedTextUtil");
const phpGlobals = require("./phpGlobals");
const phpGlobalFunctions = require("./phpGlobalFunctions");
class PHPHoverProvider {
    provideHover(document, position, _token) {
        let enable = vscode_1.workspace.getConfiguration('php').get('suggest.basic', true);
        if (!enable) {
            return undefined;
        }
        let wordRange = document.getWordRangeAtPosition(position);
        if (!wordRange) {
            return undefined;
        }
        let name = document.getText(wordRange);
        let entry = phpGlobalFunctions.globalfunctions[name] || phpGlobals.compiletimeconstants[name] || phpGlobals.globalvariables[name] || phpGlobals.keywords[name];
        if (entry && entry.description) {
            let signature = name + (entry.signature || '');
            let contents = [(0, markedTextUtil_1.textToMarkedString)(entry.description), { language: 'php', value: signature }];
            return new vscode_1.Hover(contents, wordRange);
        }
        return undefined;
    }
}
exports.default = PHPHoverProvider;
//# sourceMappingURL=hoverProvider.js.map