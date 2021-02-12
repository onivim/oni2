"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const vscode_1 = require("vscode");
const phpGlobals = require("./phpGlobals");
const phpGlobalFunctions = require("./phpGlobalFunctions");
class PHPCompletionItemProvider {
    provideCompletionItems(document, position, _token, context) {
        let result = [];
        let shouldProvideCompletionItems = vscode_1.workspace.getConfiguration('php').get('suggest.basic', true);
        if (!shouldProvideCompletionItems) {
            return Promise.resolve(result);
        }
        let range = document.getWordRangeAtPosition(position);
        let prefix = range ? document.getText(range) : '';
        if (!range) {
            range = new vscode_1.Range(position, position);
        }
        if (context.triggerCharacter === '>') {
            const twoBeforeCursor = new vscode_1.Position(position.line, Math.max(0, position.character - 2));
            const previousTwoChars = document.getText(new vscode_1.Range(twoBeforeCursor, position));
            if (previousTwoChars !== '->') {
                return Promise.resolve(result);
            }
        }
        let added = {};
        let createNewProposal = function (kind, name, entry) {
            let proposal = new vscode_1.CompletionItem(name);
            proposal.kind = kind;
            if (entry) {
                if (entry.description) {
                    proposal.documentation = entry.description;
                }
                if (entry.signature) {
                    proposal.detail = entry.signature;
                }
            }
            return proposal;
        };
        let matches = (name) => {
            return prefix.length === 0 || name.length >= prefix.length && name.substr(0, prefix.length) === prefix;
        };
        if (matches('php') && range.start.character >= 2) {
            let twoBeforePosition = new vscode_1.Position(range.start.line, range.start.character - 2);
            let beforeWord = document.getText(new vscode_1.Range(twoBeforePosition, range.start));
            if (beforeWord === '<?') {
                let proposal = createNewProposal(vscode_1.CompletionItemKind.Class, '<?php', null);
                proposal.textEdit = new vscode_1.TextEdit(new vscode_1.Range(twoBeforePosition, position), '<?php');
                result.push(proposal);
                return Promise.resolve(result);
            }
        }
        for (let globalvariables in phpGlobals.globalvariables) {
            if (phpGlobals.globalvariables.hasOwnProperty(globalvariables) && matches(globalvariables)) {
                added[globalvariables] = true;
                result.push(createNewProposal(vscode_1.CompletionItemKind.Variable, globalvariables, phpGlobals.globalvariables[globalvariables]));
            }
        }
        for (let globalfunctions in phpGlobalFunctions.globalfunctions) {
            if (phpGlobalFunctions.globalfunctions.hasOwnProperty(globalfunctions) && matches(globalfunctions)) {
                added[globalfunctions] = true;
                result.push(createNewProposal(vscode_1.CompletionItemKind.Function, globalfunctions, phpGlobalFunctions.globalfunctions[globalfunctions]));
            }
        }
        for (let compiletimeconstants in phpGlobals.compiletimeconstants) {
            if (phpGlobals.compiletimeconstants.hasOwnProperty(compiletimeconstants) && matches(compiletimeconstants)) {
                added[compiletimeconstants] = true;
                result.push(createNewProposal(vscode_1.CompletionItemKind.Field, compiletimeconstants, phpGlobals.compiletimeconstants[compiletimeconstants]));
            }
        }
        for (let keywords in phpGlobals.keywords) {
            if (phpGlobals.keywords.hasOwnProperty(keywords) && matches(keywords)) {
                added[keywords] = true;
                result.push(createNewProposal(vscode_1.CompletionItemKind.Keyword, keywords, phpGlobals.keywords[keywords]));
            }
        }
        let text = document.getText();
        if (prefix[0] === '$') {
            let variableMatch = /\$([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)/g;
            let match = null;
            while (match = variableMatch.exec(text)) {
                let word = match[0];
                if (!added[word]) {
                    added[word] = true;
                    result.push(createNewProposal(vscode_1.CompletionItemKind.Variable, word, null));
                }
            }
        }
        let functionMatch = /function\s+([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)\s*\(/g;
        let match2 = null;
        while (match2 = functionMatch.exec(text)) {
            let word2 = match2[1];
            if (!added[word2]) {
                added[word2] = true;
                result.push(createNewProposal(vscode_1.CompletionItemKind.Function, word2, null));
            }
        }
        return Promise.resolve(result);
    }
}
exports.default = PHPCompletionItemProvider;
//# sourceMappingURL=completionItemProvider.js.map