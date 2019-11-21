"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const path = require("path");
const fs = require("fs");
const nls = require("vscode-nls");
const localize = nls.loadMessageBundle();
const vscode_1 = require("vscode");
const vscode_languageclient_1 = require("vscode-languageclient");
const customData_1 = require("./customData");
// this method is called when vs code is activated
function activate(context) {
    let serverMain = readJSONFile(context.asAbsolutePath('./server/package.json')).main;
    let serverModule = context.asAbsolutePath(path.join('server', serverMain));
    // The debug options for the server
    let debugOptions = { execArgv: ['--nolazy', '--inspect=6044'] };
    // If the extension is launch in debug mode the debug server options are use
    // Otherwise the run options are used
    let serverOptions = {
        run: { module: serverModule, transport: vscode_languageclient_1.TransportKind.ipc },
        debug: { module: serverModule, transport: vscode_languageclient_1.TransportKind.ipc, options: debugOptions }
    };
    let documentSelector = ['css', 'scss', 'less'];
    let dataPaths = [
        ...customData_1.getCustomDataPathsInAllWorkspaces(vscode_1.workspace.workspaceFolders),
        ...customData_1.getCustomDataPathsFromAllExtensions()
    ];
    // Options to control the language client
    let clientOptions = {
        documentSelector,
        synchronize: {
            configurationSection: ['css', 'scss', 'less']
        },
        initializationOptions: {
            dataPaths
        }
    };
    // Create the language client and start the client.
    let client = new vscode_languageclient_1.LanguageClient('css', localize('cssserver.name', 'CSS Language Server'), serverOptions, clientOptions);
    client.registerProposedFeatures();
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
        documentSelector.forEach(selector => {
            context.subscriptions.push(vscode_1.languages.registerSelectionRangeProvider(selector, {
                async provideSelectionRanges(document, positions) {
                    const textDocument = client.code2ProtocolConverter.asTextDocumentIdentifier(document);
                    const rawResult = await client.sendRequest('$/textDocument/selectionRanges', { textDocument, positions: positions.map(client.code2ProtocolConverter.asPosition) });
                    if (Array.isArray(rawResult)) {
                        return rawResult.map(rawSelectionRanges => {
                            return rawSelectionRanges.reduceRight((parent, selectionRange) => {
                                return {
                                    range: client.protocol2CodeConverter.asRange(selectionRange.range),
                                    parent
                                };
                            }, undefined);
                        });
                    }
                    return [];
                }
            }));
        });
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
exports.activate = activate;
function readJSONFile(location) {
    try {
        return JSON.parse(fs.readFileSync(location).toString());
    }
    catch (e) {
        console.log(`Problems reading ${location}: ${e}`);
        return {};
    }
}
//# sourceMappingURL=cssMain.js.map
