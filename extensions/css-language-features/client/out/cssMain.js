"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : new P(function (resolve) { resolve(result.value); }).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (_) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
exports.__esModule = true;
var path = require("path");
var fs = require("fs");
var nls = require("vscode-nls");
var localize = nls.loadMessageBundle();
var vscode_1 = require("vscode");
var vscode_languageclient_1 = require("vscode-languageclient");
var customData_1 = require("./customData");
// this method is called when vs code is activated
function activate(context) {
    const orange = vscode_1.window.createOutputChannel("Orange");
    orange.appendLine("HI!");
    var serverMain = readJSONFile(context.asAbsolutePath('./server/package.json')).main;
    var serverModule = context.asAbsolutePath(path.join('server', serverMain));
    // The debug options for the server
    var debugOptions = { execArgv: ['--nolazy', '--inspect=6044'] };
    // If the extension is launch in debug mode the debug server options are use
    // Otherwise the run options are used
    var serverOptions = {
        run: { module: serverModule, transport: vscode_languageclient_1.TransportKind.ipc },
        debug: { module: serverModule, transport: vscode_languageclient_1.TransportKind.ipc, options: debugOptions }
    };
    var documentSelector = ['css', 'scss', 'less'];
    var dataPaths = customData_1.getCustomDataPathsInAllWorkspaces(vscode_1.workspace.workspaceFolders).concat(customData_1.getCustomDataPathsFromAllExtensions());
    // Options to control the language client
    var clientOptions = {
        documentSelector: documentSelector,
        synchronize: {
            configurationSection: ['css', 'scss', 'less']
        },
        initializationOptions: {
            dataPaths: dataPaths
        }
    };
    // Create the language client and start the client.
    var client = new vscode_languageclient_1.LanguageClient('css', localize('cssserver.name', 'CSS Language Server'), serverOptions, clientOptions);
    client.registerProposedFeatures();
    orange.appendLine("CREATED CLIENT!");
    var disposable = client.start();
    // Push the disposable to the context's subscriptions so that the
    // client can be deactivated on extension deactivation
    context.subscriptions.push(disposable);
    var indentationRules = {
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
    client.onReady().then(function () {
	orange.appendLine("ONREADY CALLED!!!");
        context.subscriptions.push(initCompletionProvider());
        documentSelector.forEach(function (selector) {
            context.subscriptions.push(vscode_1.languages.registerSelectionRangeProvider(selector, {
                provideSelectionRanges: function (document, positions) {
                    return __awaiter(this, void 0, void 0, function () {
                        var textDocument, rawResult;
                        return __generator(this, function (_a) {
                            switch (_a.label) {
                                case 0:
                                    textDocument = client.code2ProtocolConverter.asTextDocumentIdentifier(document);
                                    return [4 /*yield*/, client.sendRequest('$/textDocument/selectionRanges', { textDocument: textDocument, positions: positions.map(client.code2ProtocolConverter.asPosition) })];
                                case 1:
                                    rawResult = _a.sent();
                                    if (Array.isArray(rawResult)) {
                                        return [2 /*return*/, rawResult.map(function (rawSelectionRanges) {
                                                return rawSelectionRanges.reduceRight(function (parent, selectionRange) {
                                                    return {
                                                        range: client.protocol2CodeConverter.asRange(selectionRange.range),
                                                        parent: parent
                                                    };
                                                }, undefined);
                                            })];
                                    }
                                    return [2 /*return*/, []];
                            }
                        });
                    });
                }
            }));
        });
    });
    function initCompletionProvider() {
        var regionCompletionRegExpr = /^(\s*)(\/(\*\s*(#\w*)?)?)?$/;
        return vscode_1.languages.registerCompletionItemProvider(documentSelector, {
            provideCompletionItems: function (doc, pos) {
                var lineUntilPos = doc.getText(new vscode_1.Range(new vscode_1.Position(pos.line, 0), pos));
                var match = lineUntilPos.match(regionCompletionRegExpr);
                if (match) {
                    var range = new vscode_1.Range(new vscode_1.Position(pos.line, match[1].length), pos);
                    var beginProposal = new vscode_1.CompletionItem('#region', vscode_1.CompletionItemKind.Snippet);
                    beginProposal.range = range;
                    vscode_1.TextEdit.replace(range, '/* #region */');
                    beginProposal.insertText = new vscode_1.SnippetString('/* #region $1*/');
                    beginProposal.documentation = localize('folding.start', 'Folding Region Start');
                    beginProposal.filterText = match[2];
                    beginProposal.sortText = 'za';
                    var endProposal = new vscode_1.CompletionItem('#endregion', vscode_1.CompletionItemKind.Snippet);
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
        var textEditor = vscode_1.window.activeTextEditor;
        if (textEditor && textEditor.document.uri.toString() === uri) {
            if (textEditor.document.version !== documentVersion) {
                vscode_1.window.showInformationMessage("CSS fix is outdated and can't be applied to the document.");
            }
            textEditor.edit(function (mutator) {
                for (var _i = 0, edits_1 = edits; _i < edits_1.length; _i++) {
                    var edit = edits_1[_i];
                    mutator.replace(client.protocol2CodeConverter.asRange(edit.range), edit.newText);
                }
            }).then(function (success) {
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
        console.log("Problems reading " + location + ": " + e);
        return {};
    }
}
