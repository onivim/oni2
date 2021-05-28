"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.DefaultCompletionItemProvider = void 0;
const vscode = require("vscode");
const abbreviationActions_1 = require("./abbreviationActions");
const util_1 = require("./util");
const vscode_html_languageservice_1 = require("vscode-html-languageservice");
class DefaultCompletionItemProvider {
    constructor() {
        this.htmlLS = vscode_html_languageservice_1.getLanguageService();
    }
    provideCompletionItems(document, position, _, context) {
        const completionResult = this.provideCompletionItemsInternal(document, position, context);
        if (!completionResult) {
            this.lastCompletionType = undefined;
            return;
        }
        return completionResult.then(completionList => {
            if (!completionList || !completionList.items.length) {
                this.lastCompletionType = undefined;
                return completionList;
            }
            const item = completionList.items[0];
            const expandedText = item.documentation ? item.documentation.toString() : '';
            if (expandedText.startsWith('<')) {
                this.lastCompletionType = 'html';
            }
            else if (expandedText.indexOf(':') > 0 && expandedText.endsWith(';')) {
                this.lastCompletionType = 'css';
            }
            else {
                this.lastCompletionType = undefined;
            }
            return completionList;
        });
    }
    provideCompletionItemsInternal(document, position, context) {
        const emmetConfig = vscode.workspace.getConfiguration('emmet');
        const excludedLanguages = emmetConfig['excludeLanguages'] ? emmetConfig['excludeLanguages'] : [];
        if (excludedLanguages.indexOf(document.languageId) > -1) {
            return;
        }
        const mappedLanguages = util_1.getMappingForIncludedLanguages();
        const isSyntaxMapped = mappedLanguages[document.languageId] ? true : false;
        let syntax = util_1.getEmmetMode((isSyntaxMapped ? mappedLanguages[document.languageId] : document.languageId), excludedLanguages);
        if (!syntax
            || emmetConfig['showExpandedAbbreviation'] === 'never'
            || ((isSyntaxMapped || syntax === 'jsx') && emmetConfig['showExpandedAbbreviation'] !== 'always')) {
            return;
        }
        const helper = util_1.getEmmetHelper();
        let validateLocation = syntax === 'html' || syntax === 'jsx' || syntax === 'xml';
        let rootNode = undefined;
        let currentNode = null;
        if (document.languageId === 'html') {
            if (context.triggerKind === vscode.CompletionTriggerKind.TriggerForIncompleteCompletions) {
                switch (this.lastCompletionType) {
                    case 'html':
                        validateLocation = false;
                        break;
                    case 'css':
                        validateLocation = false;
                        syntax = 'css';
                        break;
                    default:
                        break;
                }
            }
            if (validateLocation) {
                const lsDoc = vscode_html_languageservice_1.TextDocument.create(document.uri.toString(), 'html', 0, document.getText());
                const parsedLsDoc = this.htmlLS.parseHTMLDocument(lsDoc);
                const positionOffset = document.offsetAt(position);
                const node = parsedLsDoc.findNodeAt(positionOffset);
                if (node.tag === 'script') {
                    if (node.attributes && 'type' in node.attributes) {
                        const rawTypeAttrValue = node.attributes['type'];
                        if (rawTypeAttrValue) {
                            const typeAttrValue = util_1.trimQuotes(rawTypeAttrValue);
                            if (typeAttrValue === 'application/javascript' || typeAttrValue === 'text/javascript') {
                                if (!abbreviationActions_1.getSyntaxFromArgs({ language: 'javascript' })) {
                                    return;
                                }
                                else {
                                    validateLocation = false;
                                }
                            }
                            else if (util_1.allowedMimeTypesInScriptTag.indexOf(util_1.trimQuotes(rawTypeAttrValue)) > -1) {
                                validateLocation = false;
                            }
                        }
                    }
                    else {
                        return;
                    }
                }
                else if (node.tag === 'style') {
                    syntax = 'css';
                    validateLocation = false;
                }
                else {
                    if (node.attributes && node.attributes['style']) {
                        const scanner = this.htmlLS.createScanner(document.getText(), node.start);
                        let tokenType = scanner.scan();
                        let prevAttr = undefined;
                        let styleAttrValueRange = undefined;
                        while (tokenType !== vscode_html_languageservice_1.TokenType.EOS && (scanner.getTokenEnd() <= positionOffset)) {
                            tokenType = scanner.scan();
                            if (tokenType === vscode_html_languageservice_1.TokenType.AttributeName) {
                                prevAttr = scanner.getTokenText();
                            }
                            else if (tokenType === vscode_html_languageservice_1.TokenType.AttributeValue && prevAttr === 'style') {
                                styleAttrValueRange = [scanner.getTokenOffset(), scanner.getTokenEnd()];
                            }
                        }
                        if (prevAttr === 'style' && styleAttrValueRange && positionOffset > styleAttrValueRange[0] && positionOffset < styleAttrValueRange[1]) {
                            syntax = 'css';
                            validateLocation = false;
                        }
                    }
                }
            }
        }
        const extractAbbreviationResults = helper.extractAbbreviation(document, position, !util_1.isStyleSheet(syntax));
        if (!extractAbbreviationResults || !helper.isAbbreviationValid(syntax, extractAbbreviationResults.abbreviation)) {
            return;
        }
        if (util_1.isStyleSheet(document.languageId) && context.triggerKind !== vscode.CompletionTriggerKind.TriggerForIncompleteCompletions) {
            validateLocation = true;
            let usePartialParsing = vscode.workspace.getConfiguration('emmet')['optimizeStylesheetParsing'] === true;
            rootNode = usePartialParsing && document.lineCount > 1000 ? util_1.parsePartialStylesheet(document, position) : util_1.parseDocument(document, false);
            if (!rootNode) {
                return;
            }
            currentNode = util_1.getNode(rootNode, position, true);
        }
        if (validateLocation && !abbreviationActions_1.isValidLocationForEmmetAbbreviation(document, rootNode, currentNode, syntax, position, extractAbbreviationResults.abbreviationRange)) {
            return;
        }
        let noiseCheckPromise = Promise.resolve();
        // Fix for https://github.com/Microsoft/vscode/issues/32647
        // Check for document symbols in js/ts/jsx/tsx and avoid triggering emmet for abbreviations of the form symbolName.sometext
        // Presence of > or * or + in the abbreviation denotes valid abbreviation that should trigger emmet
        if (!util_1.isStyleSheet(syntax) && (document.languageId === 'javascript' || document.languageId === 'javascriptreact' || document.languageId === 'typescript' || document.languageId === 'typescriptreact')) {
            let abbreviation = extractAbbreviationResults.abbreviation;
            if (abbreviation.startsWith('this.')) {
                noiseCheckPromise = Promise.resolve(true);
            }
            else {
                noiseCheckPromise = vscode.commands.executeCommand('vscode.executeDocumentSymbolProvider', document.uri).then((symbols) => {
                    return symbols && symbols.find(x => abbreviation === x.name || (abbreviation.startsWith(x.name + '.') && !/>|\*|\+/.test(abbreviation)));
                });
            }
        }
        return noiseCheckPromise.then((noise) => {
            if (noise) {
                return;
            }
            let result = helper.doComplete(document, position, syntax, util_1.getEmmetConfiguration(syntax));
            // https://github.com/microsoft/vscode/issues/86941
            if (result && result.items && result.items.length === 1) {
                if (result.items[0].label === 'widows: ;') {
                    return undefined;
                }
            }
            let newItems = [];
            if (result && result.items) {
                result.items.forEach((item) => {
                    let newItem = new vscode.CompletionItem(item.label);
                    newItem.documentation = item.documentation;
                    newItem.detail = item.detail;
                    newItem.insertText = new vscode.SnippetString(item.textEdit.newText);
                    let oldrange = item.textEdit.range;
                    newItem.range = new vscode.Range(oldrange.start.line, oldrange.start.character, oldrange.end.line, oldrange.end.character);
                    newItem.filterText = item.filterText;
                    newItem.sortText = item.sortText;
                    if (emmetConfig['showSuggestionsAsSnippets'] === true) {
                        newItem.kind = vscode.CompletionItemKind.Snippet;
                    }
                    newItems.push(newItem);
                });
            }
            return new vscode.CompletionList(newItems, true);
        });
    }
}
exports.DefaultCompletionItemProvider = DefaultCompletionItemProvider;
//# sourceMappingURL=defaultCompletionProvider.js.map