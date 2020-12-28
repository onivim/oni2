"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.startClient = void 0;
const vscode_1 = require("vscode");
const vscode_languageclient_1 = require("vscode-languageclient");
const nls = require("vscode-nls");
const customData_1 = require("./customData");
const requests_1 = require("./requests");
var CustomDataChangedNotification;
(function (CustomDataChangedNotification) {
    CustomDataChangedNotification.type = new vscode_languageclient_1.NotificationType('css/customDataChanged');
})(CustomDataChangedNotification || (CustomDataChangedNotification = {}));
const localize = nls.loadMessageBundle();
function startClient(context, newLanguageClient, runtime) {
    const customDataSource = customData_1.getCustomDataSource(context.subscriptions);
    let documentSelector = ['css', 'scss', 'less'];
    // Options to control the language client
    let clientOptions = {
        documentSelector,
        synchronize: {
            configurationSection: ['css', 'scss', 'less']
        },
        initializationOptions: {
            handledSchemas: ['file']
        },
        middleware: {
            provideCompletionItem(document, position, context, token, next) {
                // testing the replace / insert mode
                function updateRanges(item) {
                    const range = item.range;
                    if (range instanceof vscode_1.Range && range.end.isAfter(position) && range.start.isBeforeOrEqual(position)) {
                        item.range = { inserting: new vscode_1.Range(range.start, position), replacing: range };
                    }
                }
                function updateLabel(item) {
                    if (item.kind === vscode_1.CompletionItemKind.Color) {
                        item.label2 = {
                            name: item.label,
                            type: item.documentation
                        };
                    }
                }
                // testing the new completion
                function updateProposals(r) {
                    if (r) {
                        (Array.isArray(r) ? r : r.items).forEach(updateRanges);
                        (Array.isArray(r) ? r : r.items).forEach(updateLabel);
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
    let client = newLanguageClient('css', localize('cssserver.name', 'CSS Language Server'), clientOptions);
    client.registerProposedFeatures();
    client.onReady().then(() => {
        client.sendNotification(CustomDataChangedNotification.type, customDataSource.uris);
        customDataSource.onDidChange(() => {
            client.sendNotification(CustomDataChangedNotification.type, customDataSource.uris);
        });
        requests_1.serveFileSystemRequests(client, runtime);
    });
    let disposable = client.start();
    // Push the disposable to the context's subscriptions so that the
    // client can be deactivated on extension deactivation
    context.subscriptions.push(disposable);
    let indentationRules = {
        increaseIndentPattern: /(^.*\{[^}]*$)/,
        decreaseIndentPattern: /^\s*\}/
    };
    vscode_1.languages.setLanguageConfiguration('css', {
        wordPattern: /(#?-?\d*\.\d\w*%?)|(::?[\w-]*(?=[^,{;]*[,{]))|(([@#.!])?[\w-?]+%?|[@#!.])/g,
        indentationRules: indentationRules
    });
    vscode_1.languages.setLanguageConfiguration('less', {
        wordPattern: /(#?-?\d*\.\d\w*%?)|(::?[\w-]+(?=[^,{;]*[,{]))|(([@#.!])?[\w-?]+%?|[@#!.])/g,
        indentationRules: indentationRules
    });
    vscode_1.languages.setLanguageConfiguration('scss', {
        wordPattern: /(#?-?\d*\.\d\w*%?)|(::?[\w-]*(?=[^,{;]*[,{]))|(([@$#.!])?[\w-?]+%?|[@#!$.])/g,
        indentationRules: indentationRules
    });
    client.onReady().then(() => {
        context.subscriptions.push(initCompletionProvider());
    });
    function initCompletionProvider() {
        const regionCompletionRegExpr = /^(\s*)(\/(\*\s*(#\w*)?)?)?$/;
        return vscode_1.languages.registerCompletionItemProvider(documentSelector, {
            provideCompletionItems(doc, pos) {
                let lineUntilPos = doc.getText(new vscode_1.Range(new vscode_1.Position(pos.line, 0), pos));
                let match = lineUntilPos.match(regionCompletionRegExpr);
                if (match) {
                    let range = new vscode_1.Range(new vscode_1.Position(pos.line, match[1].length), pos);
                    let beginProposal = new vscode_1.CompletionItem('#region', vscode_1.CompletionItemKind.Snippet);
                    beginProposal.range = range;
                    vscode_1.TextEdit.replace(range, '/* #region */');
                    beginProposal.insertText = new vscode_1.SnippetString('/* #region $1*/');
                    beginProposal.documentation = localize('folding.start', 'Folding Region Start');
                    beginProposal.filterText = match[2];
                    beginProposal.sortText = 'za';
                    let endProposal = new vscode_1.CompletionItem('#endregion', vscode_1.CompletionItemKind.Snippet);
                    endProposal.range = range;
                    endProposal.insertText = '/* #endregion */';
                    endProposal.documentation = localize('folding.end', 'Folding Region End');
                    endProposal.sortText = 'zb';
                    endProposal.filterText = match[2];
                    return [beginProposal, endProposal];
                }
                return null;
            }
        });
    }
    vscode_1.commands.registerCommand('_css.applyCodeAction', applyCodeAction);
    function applyCodeAction(uri, documentVersion, edits) {
        let textEditor = vscode_1.window.activeTextEditor;
        if (textEditor && textEditor.document.uri.toString() === uri) {
            if (textEditor.document.version !== documentVersion) {
                vscode_1.window.showInformationMessage(`CSS fix is outdated and can't be applied to the document.`);
            }
            textEditor.edit(mutator => {
                for (let edit of edits) {
                    mutator.replace(client.protocol2CodeConverter.asRange(edit.range), edit.newText);
                }
            }).then(success => {
                if (!success) {
                    vscode_1.window.showErrorMessage('Failed to apply CSS fix to the document. Please consider opening an issue with steps to reproduce.');
                }
            });
        }
    }
}
exports.startClient = startClient;
//# sourceMappingURL=cssClient.js.map