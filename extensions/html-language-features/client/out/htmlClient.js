"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.startClient = void 0;
const nls = require("vscode-nls");
const localize = nls.loadMessageBundle();
const vscode_1 = require("vscode");
const vscode_languageclient_1 = require("vscode-languageclient");
const htmlEmptyTagsShared_1 = require("./htmlEmptyTagsShared");
const tagClosing_1 = require("./tagClosing");
const customData_1 = require("./customData");
var CustomDataChangedNotification;
(function (CustomDataChangedNotification) {
    CustomDataChangedNotification.type = new vscode_languageclient_1.NotificationType('html/customDataChanged');
})(CustomDataChangedNotification || (CustomDataChangedNotification = {}));
var TagCloseRequest;
(function (TagCloseRequest) {
    TagCloseRequest.type = new vscode_languageclient_1.RequestType('html/tag');
})(TagCloseRequest || (TagCloseRequest = {}));
var LinkedEditingRequest;
(function (LinkedEditingRequest) {
    LinkedEditingRequest.type = new vscode_languageclient_1.RequestType('html/linkedEditing');
})(LinkedEditingRequest || (LinkedEditingRequest = {}));
var SemanticTokenRequest;
(function (SemanticTokenRequest) {
    SemanticTokenRequest.type = new vscode_languageclient_1.RequestType('html/semanticTokens');
})(SemanticTokenRequest || (SemanticTokenRequest = {}));
var SemanticTokenLegendRequest;
(function (SemanticTokenLegendRequest) {
    SemanticTokenLegendRequest.type = new vscode_languageclient_1.RequestType0('html/semanticTokenLegend');
})(SemanticTokenLegendRequest || (SemanticTokenLegendRequest = {}));
var SettingIds;
(function (SettingIds) {
    SettingIds.linkedRename = 'editor.linkedRename';
    SettingIds.formatEnable = 'html.format.enable';
})(SettingIds || (SettingIds = {}));
function startClient(context, newLanguageClient, runtime) {
    let toDispose = context.subscriptions;
    let documentSelector = ['html', 'handlebars'];
    let embeddedLanguages = { css: true, javascript: true };
    let rangeFormatting = undefined;
    const customDataSource = customData_1.getCustomDataSource(context.subscriptions);
    // Options to control the language client
    let clientOptions = {
        documentSelector,
        synchronize: {
            configurationSection: ['html', 'css', 'javascript'], // the settings to synchronize
        },
        initializationOptions: {
            embeddedLanguages,
            handledSchemas: ['file'],
            provideFormatter: false, // tell the server to not provide formatting capability and ignore the `html.format.enable` setting.
        },
        middleware: {
            // testing the replace / insert mode
            provideCompletionItem(document, position, context, token, next) {
                function updateRanges(item) {
                    const range = item.range;
                    if (range instanceof vscode_1.Range && range.end.isAfter(position) && range.start.isBeforeOrEqual(position)) {
                        item.range = { inserting: new vscode_1.Range(range.start, position), replacing: range };
                    }
                }
                function updateProposals(r) {
                    if (r) {
                        (Array.isArray(r) ? r : r.items).forEach(updateRanges);
                    }
                    return r;
                }
                const isThenable = (obj) => obj && obj['then'];
                const r = next(document, position, context, token);
                if (isThenable(r)) {
                    return r.then(updateProposals);
                }
                return updateProposals(r);
            }
        }
    };
    // Create the language client and start the client.
    let client = newLanguageClient('html', localize('htmlserver.name', 'HTML Language Server'), clientOptions);
    client.registerProposedFeatures();
    let disposable = client.start();
    toDispose.push(disposable);
    client.onReady().then(() => {
        client.sendNotification(CustomDataChangedNotification.type, customDataSource.uris);
        customDataSource.onDidChange(() => {
            client.sendNotification(CustomDataChangedNotification.type, customDataSource.uris);
        });
        let tagRequestor = (document, position) => {
            let param = client.code2ProtocolConverter.asTextDocumentPositionParams(document, position);
            return client.sendRequest(TagCloseRequest.type, param);
        };
        disposable = tagClosing_1.activateTagClosing(tagRequestor, { html: true, handlebars: true }, 'html.autoClosingTags');
        toDispose.push(disposable);
        disposable = client.onTelemetry(e => {
            var _a;
            (_a = runtime.telemetry) === null || _a === void 0 ? void 0 : _a.sendTelemetryEvent(e.key, e.data);
        });
        toDispose.push(disposable);
        // manually register / deregister format provider based on the `html.format.enable` setting avoiding issues with late registration. See #71652.
        updateFormatterRegistration();
        toDispose.push({ dispose: () => rangeFormatting && rangeFormatting.dispose() });
        toDispose.push(vscode_1.workspace.onDidChangeConfiguration(e => e.affectsConfiguration(SettingIds.formatEnable) && updateFormatterRegistration()));
        client.sendRequest(SemanticTokenLegendRequest.type).then(legend => {
            if (legend) {
                const provider = {
                    provideDocumentSemanticTokens(doc) {
                        const params = {
                            textDocument: client.code2ProtocolConverter.asTextDocumentIdentifier(doc),
                        };
                        return client.sendRequest(SemanticTokenRequest.type, params).then(data => {
                            return data && new vscode_1.SemanticTokens(new Uint32Array(data));
                        });
                    },
                    provideDocumentRangeSemanticTokens(doc, range) {
                        const params = {
                            textDocument: client.code2ProtocolConverter.asTextDocumentIdentifier(doc),
                            ranges: [client.code2ProtocolConverter.asRange(range)]
                        };
                        return client.sendRequest(SemanticTokenRequest.type, params).then(data => {
                            return data && new vscode_1.SemanticTokens(new Uint32Array(data));
                        });
                    }
                };
                toDispose.push(vscode_1.languages.registerDocumentSemanticTokensProvider(documentSelector, provider, new vscode_1.SemanticTokensLegend(legend.types, legend.modifiers)));
            }
        });
        disposable = vscode_1.languages.registerLinkedEditingRangeProvider(documentSelector, {
            async provideLinkedEditingRanges(document, position) {
                const param = client.code2ProtocolConverter.asTextDocumentPositionParams(document, position);
                return client.sendRequest(LinkedEditingRequest.type, param).then(response => {
                    if (response) {
                        return {
                            ranges: response.map(r => client.protocol2CodeConverter.asRange(r))
                        };
                    }
                    return undefined;
                });
            }
        });
        toDispose.push(disposable);
    });
    function updateFormatterRegistration() {
        const formatEnabled = vscode_1.workspace.getConfiguration().get(SettingIds.formatEnable);
        if (!formatEnabled && rangeFormatting) {
            rangeFormatting.dispose();
            rangeFormatting = undefined;
        }
        else if (formatEnabled && !rangeFormatting) {
            rangeFormatting = vscode_1.languages.registerDocumentRangeFormattingEditProvider(documentSelector, {
                provideDocumentRangeFormattingEdits(document, range, options, token) {
                    let params = {
                        textDocument: client.code2ProtocolConverter.asTextDocumentIdentifier(document),
                        range: client.code2ProtocolConverter.asRange(range),
                        options: client.code2ProtocolConverter.asFormattingOptions(options)
                    };
                    return client.sendRequest(vscode_languageclient_1.DocumentRangeFormattingRequest.type, params, token).then(client.protocol2CodeConverter.asTextEdits, (error) => {
                        client.handleFailedRequest(vscode_languageclient_1.DocumentRangeFormattingRequest.type, error, []);
                        return Promise.resolve([]);
                    });
                }
            });
        }
    }
    vscode_1.languages.setLanguageConfiguration('html', {
        indentationRules: {
            increaseIndentPattern: /<(?!\?|(?:area|base|br|col|frame|hr|html|img|input|link|meta|param)\b|[^>]*\/>)([-_\.A-Za-z0-9]+)(?=\s|>)\b[^>]*>(?!.*<\/\1>)|<!--(?!.*-->)|\{[^}"']*$/,
            decreaseIndentPattern: /^\s*(<\/(?!html)[-_\.A-Za-z0-9]+\b[^>]*>|-->|\})/
        },
        wordPattern: /(-?\d*\.\d\w*)|([^\`\~\!\@\$\^\&\*\(\)\=\+\[\{\]\}\\\|\;\:\'\"\,\.\<\>\/\s]+)/g,
        onEnterRules: [
            {
                beforeText: new RegExp(`<(?!(?:${htmlEmptyTagsShared_1.EMPTY_ELEMENTS.join('|')}))([_:\\w][_:\\w-.\\d]*)([^/>]*(?!/)>)[^<]*$`, 'i'),
                afterText: /^<\/([_:\w][_:\w-.\d]*)\s*>/i,
                action: { indentAction: vscode_1.IndentAction.IndentOutdent }
            },
            {
                beforeText: new RegExp(`<(?!(?:${htmlEmptyTagsShared_1.EMPTY_ELEMENTS.join('|')}))(\\w[\\w\\d]*)([^/>]*(?!/)>)[^<]*$`, 'i'),
                action: { indentAction: vscode_1.IndentAction.Indent }
            }
        ],
    });
    vscode_1.languages.setLanguageConfiguration('handlebars', {
        wordPattern: /(-?\d*\.\d\w*)|([^\`\~\!\@\$\^\&\*\(\)\=\+\[\{\]\}\\\|\;\:\'\"\,\.\<\>\/\s]+)/g,
        onEnterRules: [
            {
                beforeText: new RegExp(`<(?!(?:${htmlEmptyTagsShared_1.EMPTY_ELEMENTS.join('|')}))([_:\\w][_:\\w-.\\d]*)([^/>]*(?!/)>)[^<]*$`, 'i'),
                afterText: /^<\/([_:\w][_:\w-.\d]*)\s*>/i,
                action: { indentAction: vscode_1.IndentAction.IndentOutdent }
            },
            {
                beforeText: new RegExp(`<(?!(?:${htmlEmptyTagsShared_1.EMPTY_ELEMENTS.join('|')}))(\\w[\\w\\d]*)([^/>]*(?!/)>)[^<]*$`, 'i'),
                action: { indentAction: vscode_1.IndentAction.Indent }
            }
        ],
    });
    const regionCompletionRegExpr = /^(\s*)(<(!(-(-\s*(#\w*)?)?)?)?)?$/;
    const htmlSnippetCompletionRegExpr = /^(\s*)(<(h(t(m(l)?)?)?)?)?$/;
    vscode_1.languages.registerCompletionItemProvider(documentSelector, {
        provideCompletionItems(doc, pos) {
            const results = [];
            let lineUntilPos = doc.getText(new vscode_1.Range(new vscode_1.Position(pos.line, 0), pos));
            let match = lineUntilPos.match(regionCompletionRegExpr);
            if (match) {
                let range = new vscode_1.Range(new vscode_1.Position(pos.line, match[1].length), pos);
                let beginProposal = new vscode_1.CompletionItem('#region', vscode_1.CompletionItemKind.Snippet);
                beginProposal.range = range;
                beginProposal.insertText = new vscode_1.SnippetString('<!-- #region $1-->');
                beginProposal.documentation = localize('folding.start', 'Folding Region Start');
                beginProposal.filterText = match[2];
                beginProposal.sortText = 'za';
                results.push(beginProposal);
                let endProposal = new vscode_1.CompletionItem('#endregion', vscode_1.CompletionItemKind.Snippet);
                endProposal.range = range;
                endProposal.insertText = new vscode_1.SnippetString('<!-- #endregion -->');
                endProposal.documentation = localize('folding.end', 'Folding Region End');
                endProposal.filterText = match[2];
                endProposal.sortText = 'zb';
                results.push(endProposal);
            }
            let match2 = lineUntilPos.match(htmlSnippetCompletionRegExpr);
            if (match2 && doc.getText(new vscode_1.Range(new vscode_1.Position(0, 0), pos)).match(htmlSnippetCompletionRegExpr)) {
                let range = new vscode_1.Range(new vscode_1.Position(pos.line, match2[1].length), pos);
                let snippetProposal = new vscode_1.CompletionItem('HTML sample', vscode_1.CompletionItemKind.Snippet);
                snippetProposal.range = range;
                const content = ['<!DOCTYPE html>',
                    '<html>',
                    '<head>',
                    '\t<meta charset=\'utf-8\'>',
                    '\t<meta http-equiv=\'X-UA-Compatible\' content=\'IE=edge\'>',
                    '\t<title>${1:Page Title}</title>',
                    '\t<meta name=\'viewport\' content=\'width=device-width, initial-scale=1\'>',
                    '\t<link rel=\'stylesheet\' type=\'text/css\' media=\'screen\' href=\'${2:main.css}\'>',
                    '\t<script src=\'${3:main.js}\'></script>',
                    '</head>',
                    '<body>',
                    '\t$0',
                    '</body>',
                    '</html>'].join('\n');
                snippetProposal.insertText = new vscode_1.SnippetString(content);
                snippetProposal.documentation = localize('folding.html', 'Simple HTML5 starting point');
                snippetProposal.filterText = match2[2];
                snippetProposal.sortText = 'za';
                results.push(snippetProposal);
            }
            return results;
        }
    });
    const promptForTypeOnRenameKey = 'html.promptForTypeOnRename';
    const promptForTypeOnRename = vscode_1.extensions.getExtension('formulahendry.auto-rename-tag') !== undefined &&
        (context.globalState.get(promptForTypeOnRenameKey) !== false) &&
        !vscode_1.workspace.getConfiguration('editor', { languageId: 'html' }).get('linkedRename');
    if (promptForTypeOnRename) {
        const activeEditorListener = vscode_1.window.onDidChangeActiveTextEditor(async (e) => {
            if (e && documentSelector.indexOf(e.document.languageId) !== -1) {
                context.globalState.update(promptForTypeOnRenameKey, false);
                activeEditorListener.dispose();
                const configure = localize('configureButton', 'Configure');
                const res = await vscode_1.window.showInformationMessage(localize('linkedRenameQuestion', 'VS Code now has built-in support for auto-renaming tags. Do you want to enable it?'), configure);
                if (res === configure) {
                    vscode_1.commands.executeCommand('workbench.action.openSettings', SettingIds.linkedRename);
                }
            }
        });
        toDispose.push(activeEditorListener);
    }
    toDispose.push();
}
exports.startClient = startClient;
//# sourceMappingURL=htmlClient.js.map