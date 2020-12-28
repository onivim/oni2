"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.startServer = void 0;
const vscode_languageserver_1 = require("vscode-languageserver");
const runner_1 = require("./utils/runner");
const vscode_json_languageservice_1 = require("vscode-json-languageservice");
const languageModelCache_1 = require("./languageModelCache");
const requests_1 = require("./requests");
var SchemaAssociationNotification;
(function (SchemaAssociationNotification) {
    SchemaAssociationNotification.type = new vscode_languageserver_1.NotificationType('json/schemaAssociations');
})(SchemaAssociationNotification || (SchemaAssociationNotification = {}));
var VSCodeContentRequest;
(function (VSCodeContentRequest) {
    VSCodeContentRequest.type = new vscode_languageserver_1.RequestType('vscode/content');
})(VSCodeContentRequest || (VSCodeContentRequest = {}));
var SchemaContentChangeNotification;
(function (SchemaContentChangeNotification) {
    SchemaContentChangeNotification.type = new vscode_languageserver_1.NotificationType('json/schemaContent');
})(SchemaContentChangeNotification || (SchemaContentChangeNotification = {}));
var ResultLimitReachedNotification;
(function (ResultLimitReachedNotification) {
    ResultLimitReachedNotification.type = new vscode_languageserver_1.NotificationType('json/resultLimitReached');
})(ResultLimitReachedNotification || (ResultLimitReachedNotification = {}));
var ForceValidateRequest;
(function (ForceValidateRequest) {
    ForceValidateRequest.type = new vscode_languageserver_1.RequestType('json/validate');
})(ForceValidateRequest || (ForceValidateRequest = {}));
const workspaceContext = {
    resolveRelativePath: (relativePath, resource) => {
        const base = resource.substr(0, resource.lastIndexOf('/') + 1);
        return requests_1.resolvePath(base, relativePath);
    }
};
function startServer(connection, runtime) {
    function getSchemaRequestService(handledSchemas = ['https', 'http', 'file']) {
        const builtInHandlers = {};
        for (let protocol of handledSchemas) {
            if (protocol === 'file') {
                builtInHandlers[protocol] = runtime.file;
            }
            else if (protocol === 'http' || protocol === 'https') {
                builtInHandlers[protocol] = runtime.http;
            }
        }
        return (uri) => {
            const protocol = uri.substr(0, uri.indexOf(':'));
            const builtInHandler = builtInHandlers[protocol];
            if (builtInHandler) {
                return builtInHandler.getContent(uri);
            }
            return connection.sendRequest(VSCodeContentRequest.type, uri).then(responseText => {
                return responseText;
            }, error => {
                return Promise.reject(error.message);
            });
        };
    }
    // create the JSON language service
    let languageService = vscode_json_languageservice_1.getLanguageService({
        workspaceContext,
        contributions: [],
        clientCapabilities: vscode_json_languageservice_1.ClientCapabilities.LATEST
    });
    // Create a text document manager.
    const documents = new vscode_languageserver_1.TextDocuments(vscode_json_languageservice_1.TextDocument);
    // Make the text document manager listen on the connection
    // for open, change and close text document events
    documents.listen(connection);
    let clientSnippetSupport = false;
    let dynamicFormatterRegistration = false;
    let hierarchicalDocumentSymbolSupport = false;
    let foldingRangeLimitDefault = Number.MAX_VALUE;
    let foldingRangeLimit = Number.MAX_VALUE;
    let resultLimit = Number.MAX_VALUE;
    let formatterMaxNumberOfEdits = Number.MAX_VALUE;
    // After the server has started the client sends an initialize request. The server receives
    // in the passed params the rootPath of the workspace plus the client capabilities.
    connection.onInitialize((params) => {
        var _a, _b, _c, _d, _e, _f;
        const handledProtocols = (_a = params.initializationOptions) === null || _a === void 0 ? void 0 : _a.handledSchemaProtocols;
        languageService = vscode_json_languageservice_1.getLanguageService({
            schemaRequestService: getSchemaRequestService(handledProtocols),
            workspaceContext,
            contributions: [],
            clientCapabilities: params.capabilities
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
        dynamicFormatterRegistration = getClientCapability('textDocument.rangeFormatting.dynamicRegistration', false) && (typeof ((_b = params.initializationOptions) === null || _b === void 0 ? void 0 : _b.provideFormatter) !== 'boolean');
        foldingRangeLimitDefault = getClientCapability('textDocument.foldingRange.rangeLimit', Number.MAX_VALUE);
        hierarchicalDocumentSymbolSupport = getClientCapability('textDocument.documentSymbol.hierarchicalDocumentSymbolSupport', false);
        formatterMaxNumberOfEdits = ((_e = (_d = (_c = params.initializationOptions) === null || _c === void 0 ? void 0 : _c.customCapabilities) === null || _d === void 0 ? void 0 : _d.rangeFormatting) === null || _e === void 0 ? void 0 : _e.editLimit) || Number.MAX_VALUE;
        const capabilities = {
            textDocumentSync: vscode_languageserver_1.TextDocumentSyncKind.Incremental,
            completionProvider: clientSnippetSupport ? {
                resolveProvider: false,
                triggerCharacters: ['"', ':']
            } : undefined,
            hoverProvider: true,
            documentSymbolProvider: true,
            documentRangeFormattingProvider: ((_f = params.initializationOptions) === null || _f === void 0 ? void 0 : _f.provideFormatter) === true,
            colorProvider: {},
            foldingRangeProvider: true,
            selectionRangeProvider: true,
            documentLinkProvider: {}
        };
        return { capabilities };
    });
    const limitExceededWarnings = function () {
        const pendingWarnings = {};
        return {
            cancel(uri) {
                const warning = pendingWarnings[uri];
                if (warning && warning.timeout) {
                    clearTimeout(warning.timeout);
                    delete pendingWarnings[uri];
                }
            },
            onResultLimitExceeded(uri, resultLimit, name) {
                return () => {
                    let warning = pendingWarnings[uri];
                    if (warning) {
                        if (!warning.timeout) {
                            // already shown
                            return;
                        }
                        warning.features[name] = name;
                        warning.timeout.refresh();
                    }
                    else {
                        warning = { features: { [name]: name } };
                        warning.timeout = setTimeout(() => {
                            connection.sendNotification(ResultLimitReachedNotification.type, `${requests_1.basename(uri)}: For performance reasons, ${Object.keys(warning.features).join(' and ')} have been limited to ${resultLimit} items.`);
                            warning.timeout = undefined;
                        }, 2000);
                        pendingWarnings[uri] = warning;
                    }
                };
            }
        };
    }();
    let jsonConfigurationSettings = undefined;
    let schemaAssociations = undefined;
    let formatterRegistration = null;
    // The settings have changed. Is send on server activation as well.
    connection.onDidChangeConfiguration((change) => {
        let settings = change.settings;
        if (runtime.configureHttpRequests) {
            runtime.configureHttpRequests(settings.http && settings.http.proxy, settings.http && settings.http.proxyStrictSSL);
        }
        jsonConfigurationSettings = settings.json && settings.json.schemas;
        updateConfiguration();
        foldingRangeLimit = Math.trunc(Math.max(settings.json && settings.json.resultLimit || foldingRangeLimitDefault, 0));
        resultLimit = Math.trunc(Math.max(settings.json && settings.json.resultLimit || Number.MAX_VALUE, 0));
        // dynamically enable & disable the formatter
        if (dynamicFormatterRegistration) {
            const enableFormatter = settings && settings.json && settings.json.format && settings.json.format.enable;
            if (enableFormatter) {
                if (!formatterRegistration) {
                    formatterRegistration = connection.client.register(vscode_languageserver_1.DocumentRangeFormattingRequest.type, { documentSelector: [{ language: 'json' }, { language: 'jsonc' }] });
                }
            }
            else if (formatterRegistration) {
                formatterRegistration.then(r => r.dispose());
                formatterRegistration = null;
            }
        }
    });
    // The jsonValidation extension configuration has changed
    connection.onNotification(SchemaAssociationNotification.type, associations => {
        schemaAssociations = associations;
        updateConfiguration();
    });
    // A schema has changed
    connection.onNotification(SchemaContentChangeNotification.type, uri => {
        languageService.resetSchema(uri);
    });
    // Retry schema validation on all open documents
    connection.onRequest(ForceValidateRequest.type, uri => {
        return new Promise(resolve => {
            const document = documents.get(uri);
            if (document) {
                updateConfiguration();
                validateTextDocument(document, diagnostics => {
                    resolve(diagnostics);
                });
            }
            else {
                resolve([]);
            }
        });
    });
    function updateConfiguration() {
        const languageSettings = {
            validate: true,
            allowComments: true,
            schemas: new Array()
        };
        if (schemaAssociations) {
            if (Array.isArray(schemaAssociations)) {
                Array.prototype.push.apply(languageSettings.schemas, schemaAssociations);
            }
            else {
                for (const pattern in schemaAssociations) {
                    const association = schemaAssociations[pattern];
                    if (Array.isArray(association)) {
                        association.forEach(uri => {
                            languageSettings.schemas.push({ uri, fileMatch: [pattern] });
                        });
                    }
                }
            }
        }
        if (jsonConfigurationSettings) {
            jsonConfigurationSettings.forEach((schema, index) => {
                let uri = schema.url;
                if (!uri && schema.schema) {
                    uri = schema.schema.id || `vscode://schemas/custom/${index}`;
                }
                if (uri) {
                    languageSettings.schemas.push({ uri, fileMatch: schema.fileMatch, schema: schema.schema });
                }
            });
        }
        languageService.configure(languageSettings);
        // Revalidate any open text documents
        documents.all().forEach(triggerValidation);
    }
    // The content of a text document has changed. This event is emitted
    // when the text document first opened or when its content has changed.
    documents.onDidChangeContent((change) => {
        limitExceededWarnings.cancel(change.document.uri);
        triggerValidation(change.document);
    });
    // a document has closed: clear all diagnostics
    documents.onDidClose(event => {
        limitExceededWarnings.cancel(event.document.uri);
        cleanPendingValidation(event.document);
        connection.sendDiagnostics({ uri: event.document.uri, diagnostics: [] });
    });
    const pendingValidationRequests = {};
    const validationDelayMs = 300;
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
    function validateTextDocument(textDocument, callback) {
        const respond = (diagnostics) => {
            connection.sendDiagnostics({ uri: textDocument.uri, diagnostics });
            if (callback) {
                callback(diagnostics);
            }
        };
        if (textDocument.getText().length === 0) {
            respond([]); // ignore empty documents
            return;
        }
        const jsonDocument = getJSONDocument(textDocument);
        const version = textDocument.version;
        const documentSettings = textDocument.languageId === 'jsonc' ? { comments: 'ignore', trailingCommas: 'warning' } : { comments: 'error', trailingCommas: 'error' };
        languageService.doValidation(textDocument, jsonDocument, documentSettings).then(diagnostics => {
            setImmediate(() => {
                const currDocument = documents.get(textDocument.uri);
                if (currDocument && currDocument.version === version) {
                    respond(diagnostics); // Send the computed diagnostics to VSCode.
                }
            });
        }, error => {
            connection.console.error(runner_1.formatError(`Error while validating ${textDocument.uri}`, error));
        });
    }
    connection.onDidChangeWatchedFiles((change) => {
        // Monitored files have changed in VSCode
        let hasChanges = false;
        change.changes.forEach(c => {
            if (languageService.resetSchema(c.uri)) {
                hasChanges = true;
            }
        });
        if (hasChanges) {
            documents.all().forEach(triggerValidation);
        }
    });
    const jsonDocuments = languageModelCache_1.getLanguageModelCache(10, 60, document => languageService.parseJSONDocument(document));
    documents.onDidClose(e => {
        jsonDocuments.onDocumentRemoved(e.document);
    });
    connection.onShutdown(() => {
        jsonDocuments.dispose();
    });
    function getJSONDocument(document) {
        return jsonDocuments.get(document);
    }
    connection.onCompletion((textDocumentPosition, token) => {
        return runner_1.runSafeAsync(async () => {
            const document = documents.get(textDocumentPosition.textDocument.uri);
            if (document) {
                const jsonDocument = getJSONDocument(document);
                return languageService.doComplete(document, textDocumentPosition.position, jsonDocument);
            }
            return null;
        }, null, `Error while computing completions for ${textDocumentPosition.textDocument.uri}`, token);
    });
    connection.onHover((textDocumentPositionParams, token) => {
        return runner_1.runSafeAsync(async () => {
            const document = documents.get(textDocumentPositionParams.textDocument.uri);
            if (document) {
                const jsonDocument = getJSONDocument(document);
                return languageService.doHover(document, textDocumentPositionParams.position, jsonDocument);
            }
            return null;
        }, null, `Error while computing hover for ${textDocumentPositionParams.textDocument.uri}`, token);
    });
    connection.onDocumentSymbol((documentSymbolParams, token) => {
        return runner_1.runSafe(() => {
            const document = documents.get(documentSymbolParams.textDocument.uri);
            if (document) {
                const jsonDocument = getJSONDocument(document);
                const onResultLimitExceeded = limitExceededWarnings.onResultLimitExceeded(document.uri, resultLimit, 'document symbols');
                if (hierarchicalDocumentSymbolSupport) {
                    return languageService.findDocumentSymbols2(document, jsonDocument, { resultLimit, onResultLimitExceeded });
                }
                else {
                    return languageService.findDocumentSymbols(document, jsonDocument, { resultLimit, onResultLimitExceeded });
                }
            }
            return [];
        }, [], `Error while computing document symbols for ${documentSymbolParams.textDocument.uri}`, token);
    });
    connection.onDocumentRangeFormatting((formatParams, token) => {
        return runner_1.runSafe(() => {
            const document = documents.get(formatParams.textDocument.uri);
            if (document) {
                const edits = languageService.format(document, formatParams.range, formatParams.options);
                if (edits.length > formatterMaxNumberOfEdits) {
                    const newText = vscode_json_languageservice_1.TextDocument.applyEdits(document, edits);
                    return [vscode_languageserver_1.TextEdit.replace(vscode_json_languageservice_1.Range.create(vscode_json_languageservice_1.Position.create(0, 0), document.positionAt(document.getText().length)), newText)];
                }
                return edits;
            }
            return [];
        }, [], `Error while formatting range for ${formatParams.textDocument.uri}`, token);
    });
    connection.onDocumentColor((params, token) => {
        return runner_1.runSafeAsync(async () => {
            const document = documents.get(params.textDocument.uri);
            if (document) {
                const onResultLimitExceeded = limitExceededWarnings.onResultLimitExceeded(document.uri, resultLimit, 'document colors');
                const jsonDocument = getJSONDocument(document);
                return languageService.findDocumentColors(document, jsonDocument, { resultLimit, onResultLimitExceeded });
            }
            return [];
        }, [], `Error while computing document colors for ${params.textDocument.uri}`, token);
    });
    connection.onColorPresentation((params, token) => {
        return runner_1.runSafe(() => {
            const document = documents.get(params.textDocument.uri);
            if (document) {
                const jsonDocument = getJSONDocument(document);
                return languageService.getColorPresentations(document, jsonDocument, params.color, params.range);
            }
            return [];
        }, [], `Error while computing color presentations for ${params.textDocument.uri}`, token);
    });
    connection.onFoldingRanges((params, token) => {
        return runner_1.runSafe(() => {
            const document = documents.get(params.textDocument.uri);
            if (document) {
                const onRangeLimitExceeded = limitExceededWarnings.onResultLimitExceeded(document.uri, foldingRangeLimit, 'folding ranges');
                return languageService.getFoldingRanges(document, { rangeLimit: foldingRangeLimit, onRangeLimitExceeded });
            }
            return null;
        }, null, `Error while computing folding ranges for ${params.textDocument.uri}`, token);
    });
    connection.onSelectionRanges((params, token) => {
        return runner_1.runSafe(() => {
            const document = documents.get(params.textDocument.uri);
            if (document) {
                const jsonDocument = getJSONDocument(document);
                return languageService.getSelectionRanges(document, params.positions, jsonDocument);
            }
            return [];
        }, [], `Error while computing selection ranges for ${params.textDocument.uri}`, token);
    });
    connection.onDocumentLinks((params, token) => {
        return runner_1.runSafeAsync(async () => {
            const document = documents.get(params.textDocument.uri);
            if (document) {
                const jsonDocument = getJSONDocument(document);
                return languageService.findLinks(document, jsonDocument);
            }
            return [];
        }, [], `Error while computing links for ${params.textDocument.uri}`, token);
    });
    // Listen on the connection
    connection.listen();
}
exports.startServer = startServer;
//# sourceMappingURL=jsonServer.js.map