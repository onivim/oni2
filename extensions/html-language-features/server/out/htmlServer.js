"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.startServer = void 0;
const vscode_languageserver_1 = require("vscode-languageserver");
const languageModes_1 = require("./modes/languageModes");
const formatting_1 = require("./modes/formatting");
const arrays_1 = require("./utils/arrays");
const documentContext_1 = require("./utils/documentContext");
const vscode_uri_1 = require("vscode-uri");
const runner_1 = require("./utils/runner");
const htmlFolding_1 = require("./modes/htmlFolding");
const customData_1 = require("./customData");
const selectionRanges_1 = require("./modes/selectionRanges");
const semanticTokens_1 = require("./modes/semanticTokens");
const requests_1 = require("./requests");
var CustomDataChangedNotification;
(function (CustomDataChangedNotification) {
    CustomDataChangedNotification.type = new vscode_languageserver_1.NotificationType('html/customDataChanged');
})(CustomDataChangedNotification || (CustomDataChangedNotification = {}));
var TagCloseRequest;
(function (TagCloseRequest) {
    TagCloseRequest.type = new vscode_languageserver_1.RequestType('html/tag');
})(TagCloseRequest || (TagCloseRequest = {}));
var LinkedEditingRequest;
(function (LinkedEditingRequest) {
    LinkedEditingRequest.type = new vscode_languageserver_1.RequestType('html/linkedEditing');
})(LinkedEditingRequest || (LinkedEditingRequest = {}));
var SemanticTokenRequest;
(function (SemanticTokenRequest) {
    SemanticTokenRequest.type = new vscode_languageserver_1.RequestType('html/semanticTokens');
})(SemanticTokenRequest || (SemanticTokenRequest = {}));
var SemanticTokenLegendRequest;
(function (SemanticTokenLegendRequest) {
    SemanticTokenLegendRequest.type = new vscode_languageserver_1.RequestType('html/semanticTokenLegend');
})(SemanticTokenLegendRequest || (SemanticTokenLegendRequest = {}));
function startServer(connection, runtime) {
    // Create a text document manager.
    const documents = new vscode_languageserver_1.TextDocuments(languageModes_1.TextDocument);
    // Make the text document manager listen on the connection
    // for open, change and close text document events
    documents.listen(connection);
    let workspaceFolders = [];
    let languageModes;
    let clientSnippetSupport = false;
    let dynamicFormatterRegistration = false;
    let scopedSettingsSupport = false;
    let workspaceFoldersSupport = false;
    let foldingRangeLimit = Number.MAX_VALUE;
    const notReady = () => Promise.reject('Not Ready');
    let requestService = { getContent: notReady, stat: notReady, readDirectory: notReady };
    let globalSettings = {};
    let documentSettings = {};
    // remove document settings on close
    documents.onDidClose(e => {
        delete documentSettings[e.document.uri];
    });
    function getDocumentSettings(textDocument, needsDocumentSettings) {
        if (scopedSettingsSupport && needsDocumentSettings()) {
            let promise = documentSettings[textDocument.uri];
            if (!promise) {
                const scopeUri = textDocument.uri;
                const configRequestParam = { items: [{ scopeUri, section: 'css' }, { scopeUri, section: 'html' }, { scopeUri, section: 'javascript' }] };
                promise = connection.sendRequest(vscode_languageserver_1.ConfigurationRequest.type, configRequestParam).then(s => ({ css: s[0], html: s[1], javascript: s[2] }));
                documentSettings[textDocument.uri] = promise;
            }
            return promise;
        }
        return Promise.resolve(undefined);
    }
    // After the server has started the client sends an initialize request. The server receives
    // in the passed params the rootPath of the workspace plus the client capabilities
    connection.onInitialize((params) => {
        const initializationOptions = params.initializationOptions;
        workspaceFolders = params.workspaceFolders;
        if (!Array.isArray(workspaceFolders)) {
            workspaceFolders = [];
            if (params.rootPath) {
                workspaceFolders.push({ name: '', uri: vscode_uri_1.URI.file(params.rootPath).toString() });
            }
        }
        requestService = requests_1.getRequestService((initializationOptions === null || initializationOptions === void 0 ? void 0 : initializationOptions.handledSchemas) || ['file'], connection, runtime);
        const workspace = {
            get settings() { return globalSettings; },
            get folders() { return workspaceFolders; }
        };
        languageModes = languageModes_1.getLanguageModes((initializationOptions === null || initializationOptions === void 0 ? void 0 : initializationOptions.embeddedLanguages) || { css: true, javascript: true }, workspace, params.capabilities, requestService);
        const dataPaths = (initializationOptions === null || initializationOptions === void 0 ? void 0 : initializationOptions.dataPaths) || [];
        customData_1.fetchHTMLDataProviders(dataPaths, requestService).then(dataProviders => {
            languageModes.updateDataProviders(dataProviders);
        });
        documents.onDidClose(e => {
            languageModes.onDocumentRemoved(e.document);
        });
        connection.onShutdown(() => {
            languageModes.dispose();
        });
        function getClientCapability(name, def) {
            const keys = name.split('.');
            let c = params.capabilities;
            for (let i = 0; c && i < keys.length; i++) {
                if (!c.hasOwnProperty(keys[i])) {
                    return def;
                }
                c = c[keys[i]];
            }
            return c;
        }
        clientSnippetSupport = getClientCapability('textDocument.completion.completionItem.snippetSupport', false);
        dynamicFormatterRegistration = getClientCapability('textDocument.rangeFormatting.dynamicRegistration', false) && (typeof (initializationOptions === null || initializationOptions === void 0 ? void 0 : initializationOptions.provideFormatter) !== 'boolean');
        scopedSettingsSupport = getClientCapability('workspace.configuration', false);
        workspaceFoldersSupport = getClientCapability('workspace.workspaceFolders', false);
        foldingRangeLimit = getClientCapability('textDocument.foldingRange.rangeLimit', Number.MAX_VALUE);
        const capabilities = {
            textDocumentSync: vscode_languageserver_1.TextDocumentSyncKind.Incremental,
            completionProvider: clientSnippetSupport ? { resolveProvider: true, triggerCharacters: ['.', ':', '<', '"', '=', '/'] } : undefined,
            hoverProvider: true,
            documentHighlightProvider: true,
            documentRangeFormattingProvider: (initializationOptions === null || initializationOptions === void 0 ? void 0 : initializationOptions.provideFormatter) === true,
            documentLinkProvider: { resolveProvider: false },
            documentSymbolProvider: true,
            definitionProvider: true,
            signatureHelpProvider: { triggerCharacters: ['('] },
            referencesProvider: true,
            colorProvider: {},
            foldingRangeProvider: true,
            selectionRangeProvider: true,
            renameProvider: true
        };
        return { capabilities };
    });
    connection.onInitialized(() => {
        if (workspaceFoldersSupport) {
            connection.client.register(vscode_languageserver_1.DidChangeWorkspaceFoldersNotification.type);
            connection.onNotification(vscode_languageserver_1.DidChangeWorkspaceFoldersNotification.type, e => {
                const toAdd = e.event.added;
                const toRemove = e.event.removed;
                const updatedFolders = [];
                if (workspaceFolders) {
                    for (const folder of workspaceFolders) {
                        if (!toRemove.some(r => r.uri === folder.uri) && !toAdd.some(r => r.uri === folder.uri)) {
                            updatedFolders.push(folder);
                        }
                    }
                }
                workspaceFolders = updatedFolders.concat(toAdd);
                documents.all().forEach(triggerValidation);
            });
        }
    });
    let formatterRegistration = null;
    // The settings have changed. Is send on server activation as well.
    connection.onDidChangeConfiguration((change) => {
        globalSettings = change.settings;
        documentSettings = {}; // reset all document settings
        documents.all().forEach(triggerValidation);
        // dynamically enable & disable the formatter
        if (dynamicFormatterRegistration) {
            const enableFormatter = globalSettings && globalSettings.html && globalSettings.html.format && globalSettings.html.format.enable;
            if (enableFormatter) {
                if (!formatterRegistration) {
                    const documentSelector = [{ language: 'html' }, { language: 'handlebars' }];
                    formatterRegistration = connection.client.register(vscode_languageserver_1.DocumentRangeFormattingRequest.type, { documentSelector });
                }
            }
            else if (formatterRegistration) {
                formatterRegistration.then(r => r.dispose());
                formatterRegistration = null;
            }
        }
    });
    const pendingValidationRequests = {};
    const validationDelayMs = 500;
    // The content of a text document has changed. This event is emitted
    // when the text document first opened or when its content has changed.
    documents.onDidChangeContent(change => {
        triggerValidation(change.document);
    });
    // a document has closed: clear all diagnostics
    documents.onDidClose(event => {
        cleanPendingValidation(event.document);
        connection.sendDiagnostics({ uri: event.document.uri, diagnostics: [] });
    });
    function cleanPendingValidation(textDocument) {
        const request = pendingValidationRequests[textDocument.uri];
        if (request) {
            clearTimeout(request);
            delete pendingValidationRequests[textDocument.uri];
        }
    }
    function triggerValidation(textDocument) {
        cleanPendingValidation(textDocument);
        pendingValidationRequests[textDocument.uri] = setTimeout(() => {
            delete pendingValidationRequests[textDocument.uri];
            validateTextDocument(textDocument);
        }, validationDelayMs);
    }
    function isValidationEnabled(languageId, settings = globalSettings) {
        const validationSettings = settings && settings.html && settings.html.validate;
        if (validationSettings) {
            return languageId === 'css' && validationSettings.styles !== false || languageId === 'javascript' && validationSettings.scripts !== false;
        }
        return true;
    }
    async function validateTextDocument(textDocument) {
        try {
            const version = textDocument.version;
            const diagnostics = [];
            if (textDocument.languageId === 'html') {
                const modes = languageModes.getAllModesInDocument(textDocument);
                const settings = await getDocumentSettings(textDocument, () => modes.some(m => !!m.doValidation));
                const latestTextDocument = documents.get(textDocument.uri);
                if (latestTextDocument && latestTextDocument.version === version) { // check no new version has come in after in after the async op
                    for (const mode of modes) {
                        if (mode.doValidation && isValidationEnabled(mode.getId(), settings)) {
                            arrays_1.pushAll(diagnostics, await mode.doValidation(latestTextDocument, settings));
                        }
                    }
                    connection.sendDiagnostics({ uri: latestTextDocument.uri, diagnostics });
                }
            }
        }
        catch (e) {
            connection.console.error(runner_1.formatError(`Error while validating ${textDocument.uri}`, e));
        }
    }
    connection.onCompletion(async (textDocumentPosition, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(textDocumentPosition.textDocument.uri);
            if (!document) {
                return null;
            }
            const mode = languageModes.getModeAtPosition(document, textDocumentPosition.position);
            if (!mode || !mode.doComplete) {
                return { isIncomplete: true, items: [] };
            }
            const doComplete = mode.doComplete;
            if (mode.getId() !== 'html') {
                /* __GDPR__
                    "html.embbedded.complete" : {
                        "languageId" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" }
                    }
                 */
                connection.telemetry.logEvent({ key: 'html.embbedded.complete', value: { languageId: mode.getId() } });
            }
            const settings = await getDocumentSettings(document, () => doComplete.length > 2);
            const documentContext = documentContext_1.getDocumentContext(document.uri, workspaceFolders);
            return doComplete(document, textDocumentPosition.position, documentContext, settings);
        }, null, `Error while computing completions for ${textDocumentPosition.textDocument.uri}`, token);
    });
    connection.onCompletionResolve((item, token) => {
        return runner_1.runSafe(async () => {
            const data = item.data;
            if (data && data.languageId && data.uri) {
                const mode = languageModes.getMode(data.languageId);
                const document = documents.get(data.uri);
                if (mode && mode.doResolve && document) {
                    return mode.doResolve(document, item);
                }
            }
            return item;
        }, item, `Error while resolving completion proposal`, token);
    });
    connection.onHover((textDocumentPosition, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(textDocumentPosition.textDocument.uri);
            if (document) {
                const mode = languageModes.getModeAtPosition(document, textDocumentPosition.position);
                const doHover = mode === null || mode === void 0 ? void 0 : mode.doHover;
                if (doHover) {
                    const settings = await getDocumentSettings(document, () => doHover.length > 2);
                    return doHover(document, textDocumentPosition.position, settings);
                }
            }
            return null;
        }, null, `Error while computing hover for ${textDocumentPosition.textDocument.uri}`, token);
    });
    connection.onDocumentHighlight((documentHighlightParams, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(documentHighlightParams.textDocument.uri);
            if (document) {
                const mode = languageModes.getModeAtPosition(document, documentHighlightParams.position);
                if (mode && mode.findDocumentHighlight) {
                    return mode.findDocumentHighlight(document, documentHighlightParams.position);
                }
            }
            return [];
        }, [], `Error while computing document highlights for ${documentHighlightParams.textDocument.uri}`, token);
    });
    connection.onDefinition((definitionParams, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(definitionParams.textDocument.uri);
            if (document) {
                const mode = languageModes.getModeAtPosition(document, definitionParams.position);
                if (mode && mode.findDefinition) {
                    return mode.findDefinition(document, definitionParams.position);
                }
            }
            return [];
        }, null, `Error while computing definitions for ${definitionParams.textDocument.uri}`, token);
    });
    connection.onReferences((referenceParams, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(referenceParams.textDocument.uri);
            if (document) {
                const mode = languageModes.getModeAtPosition(document, referenceParams.position);
                if (mode && mode.findReferences) {
                    return mode.findReferences(document, referenceParams.position);
                }
            }
            return [];
        }, [], `Error while computing references for ${referenceParams.textDocument.uri}`, token);
    });
    connection.onSignatureHelp((signatureHelpParms, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(signatureHelpParms.textDocument.uri);
            if (document) {
                const mode = languageModes.getModeAtPosition(document, signatureHelpParms.position);
                if (mode && mode.doSignatureHelp) {
                    return mode.doSignatureHelp(document, signatureHelpParms.position);
                }
            }
            return null;
        }, null, `Error while computing signature help for ${signatureHelpParms.textDocument.uri}`, token);
    });
    connection.onDocumentRangeFormatting(async (formatParams, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(formatParams.textDocument.uri);
            if (document) {
                let settings = await getDocumentSettings(document, () => true);
                if (!settings) {
                    settings = globalSettings;
                }
                const unformattedTags = settings && settings.html && settings.html.format && settings.html.format.unformatted || '';
                const enabledModes = { css: !unformattedTags.match(/\bstyle\b/), javascript: !unformattedTags.match(/\bscript\b/) };
                return formatting_1.format(languageModes, document, formatParams.range, formatParams.options, settings, enabledModes);
            }
            return [];
        }, [], `Error while formatting range for ${formatParams.textDocument.uri}`, token);
    });
    connection.onDocumentLinks((documentLinkParam, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(documentLinkParam.textDocument.uri);
            const links = [];
            if (document) {
                const documentContext = documentContext_1.getDocumentContext(document.uri, workspaceFolders);
                for (const m of languageModes.getAllModesInDocument(document)) {
                    if (m.findDocumentLinks) {
                        arrays_1.pushAll(links, await m.findDocumentLinks(document, documentContext));
                    }
                }
            }
            return links;
        }, [], `Error while document links for ${documentLinkParam.textDocument.uri}`, token);
    });
    connection.onDocumentSymbol((documentSymbolParms, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(documentSymbolParms.textDocument.uri);
            const symbols = [];
            if (document) {
                for (const m of languageModes.getAllModesInDocument(document)) {
                    if (m.findDocumentSymbols) {
                        arrays_1.pushAll(symbols, await m.findDocumentSymbols(document));
                    }
                }
            }
            return symbols;
        }, [], `Error while computing document symbols for ${documentSymbolParms.textDocument.uri}`, token);
    });
    connection.onRequest(vscode_languageserver_1.DocumentColorRequest.type, (params, token) => {
        return runner_1.runSafe(async () => {
            const infos = [];
            const document = documents.get(params.textDocument.uri);
            if (document) {
                for (const m of languageModes.getAllModesInDocument(document)) {
                    if (m.findDocumentColors) {
                        arrays_1.pushAll(infos, await m.findDocumentColors(document));
                    }
                }
            }
            return infos;
        }, [], `Error while computing document colors for ${params.textDocument.uri}`, token);
    });
    connection.onRequest(vscode_languageserver_1.ColorPresentationRequest.type, (params, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(params.textDocument.uri);
            if (document) {
                const mode = languageModes.getModeAtPosition(document, params.range.start);
                if (mode && mode.getColorPresentations) {
                    return mode.getColorPresentations(document, params.color, params.range);
                }
            }
            return [];
        }, [], `Error while computing color presentations for ${params.textDocument.uri}`, token);
    });
    connection.onRequest(TagCloseRequest.type, (params, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(params.textDocument.uri);
            if (document) {
                const pos = params.position;
                if (pos.character > 0) {
                    const mode = languageModes.getModeAtPosition(document, languageModes_1.Position.create(pos.line, pos.character - 1));
                    if (mode && mode.doAutoClose) {
                        return mode.doAutoClose(document, pos);
                    }
                }
            }
            return null;
        }, null, `Error while computing tag close actions for ${params.textDocument.uri}`, token);
    });
    connection.onFoldingRanges((params, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(params.textDocument.uri);
            if (document) {
                return htmlFolding_1.getFoldingRanges(languageModes, document, foldingRangeLimit, token);
            }
            return null;
        }, null, `Error while computing folding regions for ${params.textDocument.uri}`, token);
    });
    connection.onSelectionRanges((params, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(params.textDocument.uri);
            if (document) {
                return selectionRanges_1.getSelectionRanges(languageModes, document, params.positions);
            }
            return [];
        }, [], `Error while computing selection ranges for ${params.textDocument.uri}`, token);
    });
    connection.onRenameRequest((params, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(params.textDocument.uri);
            const position = params.position;
            if (document) {
                const htmlMode = languageModes.getMode('html');
                if (htmlMode && htmlMode.doRename) {
                    return htmlMode.doRename(document, position, params.newName);
                }
            }
            return null;
        }, null, `Error while computing rename for ${params.textDocument.uri}`, token);
    });
    connection.onRequest(LinkedEditingRequest.type, (params, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(params.textDocument.uri);
            if (document) {
                const pos = params.position;
                if (pos.character > 0) {
                    const mode = languageModes.getModeAtPosition(document, languageModes_1.Position.create(pos.line, pos.character - 1));
                    if (mode && mode.doLinkedEditing) {
                        return mode.doLinkedEditing(document, pos);
                    }
                }
            }
            return null;
        }, null, `Error while computing synced regions for ${params.textDocument.uri}`, token);
    });
    let semanticTokensProvider;
    function getSemanticTokenProvider() {
        if (!semanticTokensProvider) {
            semanticTokensProvider = semanticTokens_1.newSemanticTokenProvider(languageModes);
        }
        return semanticTokensProvider;
    }
    connection.onRequest(SemanticTokenRequest.type, (params, token) => {
        return runner_1.runSafe(async () => {
            const document = documents.get(params.textDocument.uri);
            if (document) {
                return getSemanticTokenProvider().getSemanticTokens(document, params.ranges);
            }
            return null;
        }, null, `Error while computing semantic tokens for ${params.textDocument.uri}`, token);
    });
    connection.onRequest(SemanticTokenLegendRequest.type, (_params, token) => {
        return runner_1.runSafe(async () => {
            return getSemanticTokenProvider().legend;
        }, null, `Error while computing semantic tokens legend`, token);
    });
    connection.onNotification(CustomDataChangedNotification.type, dataPaths => {
        customData_1.fetchHTMLDataProviders(dataPaths, requestService).then(dataProviders => {
            languageModes.updateDataProviders(dataProviders);
        });
    });
    // Listen on the connection
    connection.listen();
}
exports.startServer = startServer;
//# sourceMappingURL=htmlServer.js.map