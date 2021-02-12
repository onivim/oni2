"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.getJavaScriptMode = void 0;
const languageModelCache_1 = require("../languageModelCache");
const languageModes_1 = require("./languageModes");
const strings_1 = require("../utils/strings");
const ts = require("typescript");
const javascriptSemanticTokens_1 = require("./javascriptSemanticTokens");
const JS_WORD_REGEX = /(-?\d*\.\d\w*)|([^\`\~\!\@\#\%\^\&\*\(\)\-\=\+\[\{\]\}\\\|\;\:\'\"\,\.\<\>\/\?\s]+)/g;
function getLanguageServiceHost(scriptKind) {
    const compilerOptions = { allowNonTsExtensions: true, allowJs: true, lib: ['lib.es6.d.ts'], target: ts.ScriptTarget.Latest, moduleResolution: ts.ModuleResolutionKind.Classic, experimentalDecorators: false };
    let currentTextDocument = languageModes_1.TextDocument.create('init', 'javascript', 1, '');
    const jsLanguageService = Promise.resolve().then(() => require(/* webpackChunkName: "javascriptLibs" */ './javascriptLibs')).then(libs => {
        const host = {
            getCompilationSettings: () => compilerOptions,
            getScriptFileNames: () => [currentTextDocument.uri, 'jquery'],
            getScriptKind: (fileName) => {
                if (fileName === currentTextDocument.uri) {
                    return scriptKind;
                }
                return fileName.substr(fileName.length - 2) === 'ts' ? ts.ScriptKind.TS : ts.ScriptKind.JS;
            },
            getScriptVersion: (fileName) => {
                if (fileName === currentTextDocument.uri) {
                    return String(currentTextDocument.version);
                }
                return '1'; // default lib an jquery.d.ts are static
            },
            getScriptSnapshot: (fileName) => {
                let text = '';
                if (fileName === currentTextDocument.uri) {
                    text = currentTextDocument.getText();
                }
                else {
                    text = libs.loadLibrary(fileName);
                }
                return {
                    getText: (start, end) => text.substring(start, end),
                    getLength: () => text.length,
                    getChangeRange: () => undefined
                };
            },
            getCurrentDirectory: () => '',
            getDefaultLibFileName: (_options) => 'es6'
        };
        return ts.createLanguageService(host);
    });
    return {
        async getLanguageService(jsDocument) {
            currentTextDocument = jsDocument;
            return jsLanguageService;
        },
        getCompilationSettings() {
            return compilerOptions;
        },
        dispose() {
            if (jsLanguageService) {
                jsLanguageService.then(s => s.dispose());
            }
        }
    };
}
function getJavaScriptMode(documentRegions, languageId, workspace) {
    let jsDocuments = languageModelCache_1.getLanguageModelCache(10, 60, document => documentRegions.get(document).getEmbeddedDocument(languageId));
    const host = getLanguageServiceHost(languageId === 'javascript' ? ts.ScriptKind.JS : ts.ScriptKind.TS);
    let globalSettings = {};
    return {
        getId() {
            return languageId;
        },
        async doValidation(document, settings = workspace.settings) {
            host.getCompilationSettings()['experimentalDecorators'] = settings && settings.javascript && settings.javascript.implicitProjectConfig.experimentalDecorators;
            const jsDocument = jsDocuments.get(document);
            const languageService = await host.getLanguageService(jsDocument);
            const syntaxDiagnostics = languageService.getSyntacticDiagnostics(jsDocument.uri);
            const semanticDiagnostics = languageService.getSemanticDiagnostics(jsDocument.uri);
            return syntaxDiagnostics.concat(semanticDiagnostics).map((diag) => {
                return {
                    range: convertRange(jsDocument, diag),
                    severity: languageModes_1.DiagnosticSeverity.Error,
                    source: languageId,
                    message: ts.flattenDiagnosticMessageText(diag.messageText, '\n')
                };
            });
        },
        async doComplete(document, position, _documentContext) {
            const jsDocument = jsDocuments.get(document);
            const jsLanguageService = await host.getLanguageService(jsDocument);
            let offset = jsDocument.offsetAt(position);
            let completions = jsLanguageService.getCompletionsAtPosition(jsDocument.uri, offset, { includeExternalModuleExports: false, includeInsertTextCompletions: false });
            if (!completions) {
                return { isIncomplete: false, items: [] };
            }
            let replaceRange = convertRange(jsDocument, strings_1.getWordAtText(jsDocument.getText(), offset, JS_WORD_REGEX));
            return {
                isIncomplete: false,
                items: completions.entries.map(entry => {
                    return {
                        uri: document.uri,
                        position: position,
                        label: entry.name,
                        sortText: entry.sortText,
                        kind: convertKind(entry.kind),
                        textEdit: languageModes_1.TextEdit.replace(replaceRange, entry.name),
                        data: {
                            languageId,
                            uri: document.uri,
                            offset: offset
                        }
                    };
                })
            };
        },
        async doResolve(document, item) {
            const jsDocument = jsDocuments.get(document);
            const jsLanguageService = await host.getLanguageService(jsDocument);
            let details = jsLanguageService.getCompletionEntryDetails(jsDocument.uri, item.data.offset, item.label, undefined, undefined, undefined);
            if (details) {
                item.detail = ts.displayPartsToString(details.displayParts);
                item.documentation = ts.displayPartsToString(details.documentation);
                delete item.data;
            }
            return item;
        },
        async doHover(document, position) {
            const jsDocument = jsDocuments.get(document);
            const jsLanguageService = await host.getLanguageService(jsDocument);
            let info = jsLanguageService.getQuickInfoAtPosition(jsDocument.uri, jsDocument.offsetAt(position));
            if (info) {
                const contents = ts.displayPartsToString(info.displayParts);
                return {
                    range: convertRange(jsDocument, info.textSpan),
                    contents: ['```typescript', contents, '```'].join('\n')
                };
            }
            return null;
        },
        async doSignatureHelp(document, position) {
            const jsDocument = jsDocuments.get(document);
            const jsLanguageService = await host.getLanguageService(jsDocument);
            let signHelp = jsLanguageService.getSignatureHelpItems(jsDocument.uri, jsDocument.offsetAt(position), undefined);
            if (signHelp) {
                let ret = {
                    activeSignature: signHelp.selectedItemIndex,
                    activeParameter: signHelp.argumentIndex,
                    signatures: []
                };
                signHelp.items.forEach(item => {
                    let signature = {
                        label: '',
                        documentation: undefined,
                        parameters: []
                    };
                    signature.label += ts.displayPartsToString(item.prefixDisplayParts);
                    item.parameters.forEach((p, i, a) => {
                        let label = ts.displayPartsToString(p.displayParts);
                        let parameter = {
                            label: label,
                            documentation: ts.displayPartsToString(p.documentation)
                        };
                        signature.label += label;
                        signature.parameters.push(parameter);
                        if (i < a.length - 1) {
                            signature.label += ts.displayPartsToString(item.separatorDisplayParts);
                        }
                    });
                    signature.label += ts.displayPartsToString(item.suffixDisplayParts);
                    ret.signatures.push(signature);
                });
                return ret;
            }
            return null;
        },
        async findDocumentHighlight(document, position) {
            const jsDocument = jsDocuments.get(document);
            const jsLanguageService = await host.getLanguageService(jsDocument);
            const highlights = jsLanguageService.getDocumentHighlights(jsDocument.uri, jsDocument.offsetAt(position), [jsDocument.uri]);
            const out = [];
            for (const entry of highlights || []) {
                for (const highlight of entry.highlightSpans) {
                    out.push({
                        range: convertRange(jsDocument, highlight.textSpan),
                        kind: highlight.kind === 'writtenReference' ? languageModes_1.DocumentHighlightKind.Write : languageModes_1.DocumentHighlightKind.Text
                    });
                }
            }
            return out;
        },
        async findDocumentSymbols(document) {
            const jsDocument = jsDocuments.get(document);
            const jsLanguageService = await host.getLanguageService(jsDocument);
            let items = jsLanguageService.getNavigationBarItems(jsDocument.uri);
            if (items) {
                let result = [];
                let existing = Object.create(null);
                let collectSymbols = (item, containerLabel) => {
                    let sig = item.text + item.kind + item.spans[0].start;
                    if (item.kind !== 'script' && !existing[sig]) {
                        let symbol = {
                            name: item.text,
                            kind: convertSymbolKind(item.kind),
                            location: {
                                uri: document.uri,
                                range: convertRange(jsDocument, item.spans[0])
                            },
                            containerName: containerLabel
                        };
                        existing[sig] = true;
                        result.push(symbol);
                        containerLabel = item.text;
                    }
                    if (item.childItems && item.childItems.length > 0) {
                        for (let child of item.childItems) {
                            collectSymbols(child, containerLabel);
                        }
                    }
                };
                items.forEach(item => collectSymbols(item));
                return result;
            }
            return [];
        },
        async findDefinition(document, position) {
            const jsDocument = jsDocuments.get(document);
            const jsLanguageService = await host.getLanguageService(jsDocument);
            let definition = jsLanguageService.getDefinitionAtPosition(jsDocument.uri, jsDocument.offsetAt(position));
            if (definition) {
                return definition.filter(d => d.fileName === jsDocument.uri).map(d => {
                    return {
                        uri: document.uri,
                        range: convertRange(jsDocument, d.textSpan)
                    };
                });
            }
            return null;
        },
        async findReferences(document, position) {
            const jsDocument = jsDocuments.get(document);
            const jsLanguageService = await host.getLanguageService(jsDocument);
            let references = jsLanguageService.getReferencesAtPosition(jsDocument.uri, jsDocument.offsetAt(position));
            if (references) {
                return references.filter(d => d.fileName === jsDocument.uri).map(d => {
                    return {
                        uri: document.uri,
                        range: convertRange(jsDocument, d.textSpan)
                    };
                });
            }
            return [];
        },
        async getSelectionRange(document, position) {
            const jsDocument = jsDocuments.get(document);
            const jsLanguageService = await host.getLanguageService(jsDocument);
            function convertSelectionRange(selectionRange) {
                const parent = selectionRange.parent ? convertSelectionRange(selectionRange.parent) : undefined;
                return languageModes_1.SelectionRange.create(convertRange(jsDocument, selectionRange.textSpan), parent);
            }
            const range = jsLanguageService.getSmartSelectionRange(jsDocument.uri, jsDocument.offsetAt(position));
            return convertSelectionRange(range);
        },
        async format(document, range, formatParams, settings = globalSettings) {
            const jsDocument = documentRegions.get(document).getEmbeddedDocument('javascript', true);
            const jsLanguageService = await host.getLanguageService(jsDocument);
            let formatterSettings = settings && settings.javascript && settings.javascript.format;
            let initialIndentLevel = computeInitialIndent(document, range, formatParams);
            let formatSettings = convertOptions(formatParams, formatterSettings, initialIndentLevel + 1);
            let start = jsDocument.offsetAt(range.start);
            let end = jsDocument.offsetAt(range.end);
            let lastLineRange = null;
            if (range.end.line > range.start.line && (range.end.character === 0 || strings_1.isWhitespaceOnly(jsDocument.getText().substr(end - range.end.character, range.end.character)))) {
                end -= range.end.character;
                lastLineRange = languageModes_1.Range.create(languageModes_1.Position.create(range.end.line, 0), range.end);
            }
            let edits = jsLanguageService.getFormattingEditsForRange(jsDocument.uri, start, end, formatSettings);
            if (edits) {
                let result = [];
                for (let edit of edits) {
                    if (edit.span.start >= start && edit.span.start + edit.span.length <= end) {
                        result.push({
                            range: convertRange(jsDocument, edit.span),
                            newText: edit.newText
                        });
                    }
                }
                if (lastLineRange) {
                    result.push({
                        range: lastLineRange,
                        newText: generateIndent(initialIndentLevel, formatParams)
                    });
                }
                return result;
            }
            return [];
        },
        async getFoldingRanges(document) {
            const jsDocument = jsDocuments.get(document);
            const jsLanguageService = await host.getLanguageService(jsDocument);
            let spans = jsLanguageService.getOutliningSpans(jsDocument.uri);
            let ranges = [];
            for (let span of spans) {
                let curr = convertRange(jsDocument, span.textSpan);
                let startLine = curr.start.line;
                let endLine = curr.end.line;
                if (startLine < endLine) {
                    let foldingRange = { startLine, endLine };
                    let match = document.getText(curr).match(/^\s*\/(?:(\/\s*#(?:end)?region\b)|(\*|\/))/);
                    if (match) {
                        foldingRange.kind = match[1] ? languageModes_1.FoldingRangeKind.Region : languageModes_1.FoldingRangeKind.Comment;
                    }
                    ranges.push(foldingRange);
                }
            }
            return ranges;
        },
        onDocumentRemoved(document) {
            jsDocuments.onDocumentRemoved(document);
        },
        async getSemanticTokens(document) {
            const jsDocument = jsDocuments.get(document);
            const jsLanguageService = await host.getLanguageService(jsDocument);
            return javascriptSemanticTokens_1.getSemanticTokens(jsLanguageService, jsDocument, jsDocument.uri);
        },
        getSemanticTokenLegend() {
            return javascriptSemanticTokens_1.getSemanticTokenLegend();
        },
        dispose() {
            host.dispose();
            jsDocuments.dispose();
        }
    };
}
exports.getJavaScriptMode = getJavaScriptMode;
function convertRange(document, span) {
    if (typeof span.start === 'undefined') {
        const pos = document.positionAt(0);
        return languageModes_1.Range.create(pos, pos);
    }
    const startPosition = document.positionAt(span.start);
    const endPosition = document.positionAt(span.start + (span.length || 0));
    return languageModes_1.Range.create(startPosition, endPosition);
}
function convertKind(kind) {
    switch (kind) {
        case 'primitive type':
        case 'keyword':
            return languageModes_1.CompletionItemKind.Keyword;
        case 'var':
        case 'local var':
            return languageModes_1.CompletionItemKind.Variable;
        case 'property':
        case 'getter':
        case 'setter':
            return languageModes_1.CompletionItemKind.Field;
        case 'function':
        case 'method':
        case 'construct':
        case 'call':
        case 'index':
            return languageModes_1.CompletionItemKind.Function;
        case 'enum':
            return languageModes_1.CompletionItemKind.Enum;
        case 'module':
            return languageModes_1.CompletionItemKind.Module;
        case 'class':
            return languageModes_1.CompletionItemKind.Class;
        case 'interface':
            return languageModes_1.CompletionItemKind.Interface;
        case 'warning':
            return languageModes_1.CompletionItemKind.File;
    }
    return languageModes_1.CompletionItemKind.Property;
}
function convertSymbolKind(kind) {
    switch (kind) {
        case 'var':
        case 'local var':
        case 'const':
            return languageModes_1.SymbolKind.Variable;
        case 'function':
        case 'local function':
            return languageModes_1.SymbolKind.Function;
        case 'enum':
            return languageModes_1.SymbolKind.Enum;
        case 'module':
            return languageModes_1.SymbolKind.Module;
        case 'class':
            return languageModes_1.SymbolKind.Class;
        case 'interface':
            return languageModes_1.SymbolKind.Interface;
        case 'method':
            return languageModes_1.SymbolKind.Method;
        case 'property':
        case 'getter':
        case 'setter':
            return languageModes_1.SymbolKind.Property;
    }
    return languageModes_1.SymbolKind.Variable;
}
function convertOptions(options, formatSettings, initialIndentLevel) {
    return {
        ConvertTabsToSpaces: options.insertSpaces,
        TabSize: options.tabSize,
        IndentSize: options.tabSize,
        IndentStyle: ts.IndentStyle.Smart,
        NewLineCharacter: '\n',
        BaseIndentSize: options.tabSize * initialIndentLevel,
        InsertSpaceAfterCommaDelimiter: Boolean(!formatSettings || formatSettings.insertSpaceAfterCommaDelimiter),
        InsertSpaceAfterSemicolonInForStatements: Boolean(!formatSettings || formatSettings.insertSpaceAfterSemicolonInForStatements),
        InsertSpaceBeforeAndAfterBinaryOperators: Boolean(!formatSettings || formatSettings.insertSpaceBeforeAndAfterBinaryOperators),
        InsertSpaceAfterKeywordsInControlFlowStatements: Boolean(!formatSettings || formatSettings.insertSpaceAfterKeywordsInControlFlowStatements),
        InsertSpaceAfterFunctionKeywordForAnonymousFunctions: Boolean(!formatSettings || formatSettings.insertSpaceAfterFunctionKeywordForAnonymousFunctions),
        InsertSpaceAfterOpeningAndBeforeClosingNonemptyParenthesis: Boolean(formatSettings && formatSettings.insertSpaceAfterOpeningAndBeforeClosingNonemptyParenthesis),
        InsertSpaceAfterOpeningAndBeforeClosingNonemptyBrackets: Boolean(formatSettings && formatSettings.insertSpaceAfterOpeningAndBeforeClosingNonemptyBrackets),
        InsertSpaceAfterOpeningAndBeforeClosingNonemptyBraces: Boolean(formatSettings && formatSettings.insertSpaceAfterOpeningAndBeforeClosingNonemptyBraces),
        InsertSpaceAfterOpeningAndBeforeClosingTemplateStringBraces: Boolean(formatSettings && formatSettings.insertSpaceAfterOpeningAndBeforeClosingTemplateStringBraces),
        PlaceOpenBraceOnNewLineForControlBlocks: Boolean(formatSettings && formatSettings.placeOpenBraceOnNewLineForFunctions),
        PlaceOpenBraceOnNewLineForFunctions: Boolean(formatSettings && formatSettings.placeOpenBraceOnNewLineForControlBlocks)
    };
}
function computeInitialIndent(document, range, options) {
    let lineStart = document.offsetAt(languageModes_1.Position.create(range.start.line, 0));
    let content = document.getText();
    let i = lineStart;
    let nChars = 0;
    let tabSize = options.tabSize || 4;
    while (i < content.length) {
        let ch = content.charAt(i);
        if (ch === ' ') {
            nChars++;
        }
        else if (ch === '\t') {
            nChars += tabSize;
        }
        else {
            break;
        }
        i++;
    }
    return Math.floor(nChars / tabSize);
}
function generateIndent(level, options) {
    if (options.insertSpaces) {
        return strings_1.repeat(' ', level * options.tabSize);
    }
    else {
        return strings_1.repeat('\t', level);
    }
}
//# sourceMappingURL=javascriptMode.js.map