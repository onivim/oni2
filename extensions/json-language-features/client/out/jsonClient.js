"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.startClient = void 0;
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
const nls = require("vscode-nls");
const localize = nls.loadMessageBundle();
const vscode_1 = require("vscode");
const vscode_languageclient_1 = require("vscode-languageclient");
const hash_1 = require("./utils/hash");
const requests_1 = require("./requests");
var VSCodeContentRequest;
(function (VSCodeContentRequest) {
    VSCodeContentRequest.type = new vscode_languageclient_1.RequestType('vscode/content');
})(VSCodeContentRequest || (VSCodeContentRequest = {}));
var SchemaContentChangeNotification;
(function (SchemaContentChangeNotification) {
    SchemaContentChangeNotification.type = new vscode_languageclient_1.NotificationType('json/schemaContent');
})(SchemaContentChangeNotification || (SchemaContentChangeNotification = {}));
var ForceValidateRequest;
(function (ForceValidateRequest) {
    ForceValidateRequest.type = new vscode_languageclient_1.RequestType('json/validate');
})(ForceValidateRequest || (ForceValidateRequest = {}));
var SchemaAssociationNotification;
(function (SchemaAssociationNotification) {
    SchemaAssociationNotification.type = new vscode_languageclient_1.NotificationType('json/schemaAssociations');
})(SchemaAssociationNotification || (SchemaAssociationNotification = {}));
var ResultLimitReachedNotification;
(function (ResultLimitReachedNotification) {
    ResultLimitReachedNotification.type = new vscode_languageclient_1.NotificationType('json/resultLimitReached');
})(ResultLimitReachedNotification || (ResultLimitReachedNotification = {}));
var SettingIds;
(function (SettingIds) {
    SettingIds.enableFormatter = 'json.format.enable';
    SettingIds.enableSchemaDownload = 'json.schemaDownload.enable';
    SettingIds.maxItemsComputed = 'json.maxItemsComputed';
})(SettingIds || (SettingIds = {}));
var StorageIds;
(function (StorageIds) {
    StorageIds.maxItemsExceededInformation = 'json.maxItemsExceededInformation';
})(StorageIds || (StorageIds = {}));
function startClient(context, newLanguageClient, runtime) {
    const toDispose = context.subscriptions;
    let rangeFormatting = undefined;
    const documentSelector = ['json', 'jsonc'];
    const schemaResolutionErrorStatusBarItem = vscode_1.window.createStatusBarItem({
        id: 'status.json.resolveError',
        name: localize('json.resolveError', "JSON: Schema Resolution Error"),
        alignment: vscode_1.StatusBarAlignment.Right,
        priority: 0,
    });
    schemaResolutionErrorStatusBarItem.text = '$(alert)';
    toDispose.push(schemaResolutionErrorStatusBarItem);
    const fileSchemaErrors = new Map();
    let schemaDownloadEnabled = true;
    // Options to control the language client
    const clientOptions = {
        // Register the server for json documents
        documentSelector,
        initializationOptions: {
            handledSchemaProtocols: ['file'],
            provideFormatter: false,
            customCapabilities: { rangeFormatting: { editLimit: 10000 } }
        },
        synchronize: {
            // Synchronize the setting section 'json' to the server
            configurationSection: ['json', 'http'],
            fileEvents: vscode_1.workspace.createFileSystemWatcher('**/*.json')
        },
        middleware: {
            workspace: {
                didChangeConfiguration: () => client.sendNotification(vscode_languageclient_1.DidChangeConfigurationNotification.type, { settings: getSettings() })
            },
            handleDiagnostics: (uri, diagnostics, next) => {
                const schemaErrorIndex = diagnostics.findIndex(isSchemaResolveError);
                if (schemaErrorIndex === -1) {
                    fileSchemaErrors.delete(uri.toString());
                    return next(uri, diagnostics);
                }
                const schemaResolveDiagnostic = diagnostics[schemaErrorIndex];
                fileSchemaErrors.set(uri.toString(), schemaResolveDiagnostic.message);
                if (!schemaDownloadEnabled) {
                    diagnostics = diagnostics.filter(d => !isSchemaResolveError(d));
                }
                if (vscode_1.window.activeTextEditor && vscode_1.window.activeTextEditor.document.uri.toString() === uri.toString()) {
                    schemaResolutionErrorStatusBarItem.show();
                }
                next(uri, diagnostics);
            },
            // testing the replace / insert mode
            provideCompletionItem(document, position, context, token, next) {
                function update(item) {
                    const range = item.range;
                    if (range instanceof vscode_1.Range && range.end.isAfter(position) && range.start.isBeforeOrEqual(position)) {
                        item.range = { inserting: new vscode_1.Range(range.start, position), replacing: range };
                    }
                    if (item.documentation instanceof vscode_1.MarkdownString) {
                        item.documentation = updateMarkdownString(item.documentation);
                    }
                }
                function updateProposals(r) {
                    if (r) {
                        (Array.isArray(r) ? r : r.items).forEach(update);
                    }
                    return r;
                }
                const r = next(document, position, context, token);
                if (isThenable(r)) {
                    return r.then(updateProposals);
                }
                return updateProposals(r);
            },
            provideHover(document, position, token, next) {
                function updateHover(r) {
                    if (r && Array.isArray(r.contents)) {
                        r.contents = r.contents.map(h => h instanceof vscode_1.MarkdownString ? updateMarkdownString(h) : h);
                    }
                    return r;
                }
                const r = next(document, position, token);
                if (isThenable(r)) {
                    return r.then(updateHover);
                }
                return updateHover(r);
            }
        }
    };
    // Create the language client and start the client.
    const client = newLanguageClient('json', localize('jsonserver.name', 'JSON Language Server'), clientOptions);
    client.registerProposedFeatures();
    const disposable = client.start();
    toDispose.push(disposable);
    client.onReady().then(() => {
        const schemaDocuments = {};
        // handle content request
        client.onRequest(VSCodeContentRequest.type, (uriPath) => {
            const uri = vscode_1.Uri.parse(uriPath);
            if (uri.scheme === 'untitled') {
                return Promise.reject(new vscode_languageclient_1.ResponseError(3, localize('untitled.schema', 'Unable to load {0}', uri.toString())));
            }
            if (uri.scheme !== 'http' && uri.scheme !== 'https') {
                return vscode_1.workspace.openTextDocument(uri).then(doc => {
                    schemaDocuments[uri.toString()] = true;
                    return doc.getText();
                }, error => {
                    return Promise.reject(new vscode_languageclient_1.ResponseError(2, error.toString()));
                });
            }
            else if (schemaDownloadEnabled) {
                if (runtime.telemetry && uri.authority === 'schema.management.azure.com') {
                    /* __GDPR__
                        "json.schema" : {
                            "schemaURL" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" }
                        }
                     */
                    runtime.telemetry.sendTelemetryEvent('json.schema', { schemaURL: uriPath });
                }
                return runtime.http.getContent(uriPath);
            }
            else {
                return Promise.reject(new vscode_languageclient_1.ResponseError(1, localize('schemaDownloadDisabled', 'Downloading schemas is disabled through setting \'{0}\'', SettingIds.enableSchemaDownload)));
            }
        });
        const handleContentChange = (uriString) => {
            if (schemaDocuments[uriString]) {
                client.sendNotification(SchemaContentChangeNotification.type, uriString);
                return true;
            }
            return false;
        };
        const handleActiveEditorChange = (activeEditor) => {
            if (!activeEditor) {
                return;
            }
            const activeDocUri = activeEditor.document.uri.toString();
            if (activeDocUri && fileSchemaErrors.has(activeDocUri)) {
                schemaResolutionErrorStatusBarItem.show();
            }
            else {
                schemaResolutionErrorStatusBarItem.hide();
            }
        };
        toDispose.push(vscode_1.workspace.onDidChangeTextDocument(e => handleContentChange(e.document.uri.toString())));
        toDispose.push(vscode_1.workspace.onDidCloseTextDocument(d => {
            const uriString = d.uri.toString();
            if (handleContentChange(uriString)) {
                delete schemaDocuments[uriString];
            }
            fileSchemaErrors.delete(uriString);
        }));
        toDispose.push(vscode_1.window.onDidChangeActiveTextEditor(handleActiveEditorChange));
        const handleRetryResolveSchemaCommand = () => {
            if (vscode_1.window.activeTextEditor) {
                schemaResolutionErrorStatusBarItem.text = '$(watch)';
                const activeDocUri = vscode_1.window.activeTextEditor.document.uri.toString();
                client.sendRequest(ForceValidateRequest.type, activeDocUri).then((diagnostics) => {
                    const schemaErrorIndex = diagnostics.findIndex(isSchemaResolveError);
                    if (schemaErrorIndex !== -1) {
                        // Show schema resolution errors in status bar only; ref: #51032
                        const schemaResolveDiagnostic = diagnostics[schemaErrorIndex];
                        fileSchemaErrors.set(activeDocUri, schemaResolveDiagnostic.message);
                    }
                    else {
                        schemaResolutionErrorStatusBarItem.hide();
                    }
                    schemaResolutionErrorStatusBarItem.text = '$(alert)';
                });
            }
        };
        toDispose.push(vscode_1.commands.registerCommand('_json.retryResolveSchema', handleRetryResolveSchemaCommand));
        client.sendNotification(SchemaAssociationNotification.type, getSchemaAssociations(context));
        vscode_1.extensions.onDidChange(_ => {
            client.sendNotification(SchemaAssociationNotification.type, getSchemaAssociations(context));
        });
        // manually register / deregister format provider based on the `json.format.enable` setting avoiding issues with late registration. See #71652.
        updateFormatterRegistration();
        toDispose.push({ dispose: () => rangeFormatting && rangeFormatting.dispose() });
        updateSchemaDownloadSetting();
        toDispose.push(vscode_1.workspace.onDidChangeConfiguration(e => {
            if (e.affectsConfiguration(SettingIds.enableFormatter)) {
                updateFormatterRegistration();
            }
            else if (e.affectsConfiguration(SettingIds.enableSchemaDownload)) {
                updateSchemaDownloadSetting();
            }
        }));
        client.onNotification(ResultLimitReachedNotification.type, async (message) => {
            const shouldPrompt = context.globalState.get(StorageIds.maxItemsExceededInformation) !== false;
            if (shouldPrompt) {
                const ok = localize('ok', "Ok");
                const openSettings = localize('goToSetting', 'Open Settings');
                const neverAgain = localize('yes never again', "Don't Show Again");
                const pick = await vscode_1.window.showInformationMessage(`${message}\n${localize('configureLimit', 'Use setting \'{0}\' to configure the limit.', SettingIds.maxItemsComputed)}`, ok, openSettings, neverAgain);
                if (pick === neverAgain) {
                    await context.globalState.update(StorageIds.maxItemsExceededInformation, false);
                }
                else if (pick === openSettings) {
                    await vscode_1.commands.executeCommand('workbench.action.openSettings', SettingIds.maxItemsComputed);
                }
            }
        });
        function updateFormatterRegistration() {
            const formatEnabled = vscode_1.workspace.getConfiguration().get(SettingIds.enableFormatter);
            if (!formatEnabled && rangeFormatting) {
                rangeFormatting.dispose();
                rangeFormatting = undefined;
            }
            else if (formatEnabled && !rangeFormatting) {
                rangeFormatting = vscode_1.languages.registerDocumentRangeFormattingEditProvider(documentSelector, {
                    provideDocumentRangeFormattingEdits(document, range, options, token) {
                        const params = {
                            textDocument: client.code2ProtocolConverter.asTextDocumentIdentifier(document),
                            range: client.code2ProtocolConverter.asRange(range),
                            options: client.code2ProtocolConverter.asFormattingOptions(options)
                        };
                        params.options.insertFinalNewline = vscode_1.workspace.getConfiguration('files', document).get('insertFinalNewline');
                        return client.sendRequest(vscode_languageclient_1.DocumentRangeFormattingRequest.type, params, token).then(client.protocol2CodeConverter.asTextEdits, (error) => {
                            client.handleFailedRequest(vscode_languageclient_1.DocumentRangeFormattingRequest.type, error, []);
                            return Promise.resolve([]);
                        });
                    }
                });
            }
        }
        function updateSchemaDownloadSetting() {
            schemaDownloadEnabled = vscode_1.workspace.getConfiguration().get(SettingIds.enableSchemaDownload) !== false;
            if (schemaDownloadEnabled) {
                schemaResolutionErrorStatusBarItem.tooltip = localize('json.schemaResolutionErrorMessage', 'Unable to resolve schema. Click to retry.');
                schemaResolutionErrorStatusBarItem.command = '_json.retryResolveSchema';
                handleRetryResolveSchemaCommand();
            }
            else {
                schemaResolutionErrorStatusBarItem.tooltip = localize('json.schemaResolutionDisabledMessage', 'Downloading schemas is disabled. Click to configure.');
                schemaResolutionErrorStatusBarItem.command = { command: 'workbench.action.openSettings', arguments: [SettingIds.enableSchemaDownload], title: '' };
            }
        }
    });
    const languageConfiguration = {
        wordPattern: /("(?:[^\\\"]*(?:\\.)?)*"?)|[^\s{}\[\],:]+/,
        indentationRules: {
            increaseIndentPattern: /({+(?=([^"]*"[^"]*")*[^"}]*$))|(\[+(?=([^"]*"[^"]*")*[^"\]]*$))/,
            decreaseIndentPattern: /^\s*[}\]],?\s*$/
        }
    };
    vscode_1.languages.setLanguageConfiguration('json', languageConfiguration);
    vscode_1.languages.setLanguageConfiguration('jsonc', languageConfiguration);
}
exports.startClient = startClient;
function getSchemaAssociations(_context) {
    const associations = [];
    vscode_1.extensions.all.forEach(extension => {
        const packageJSON = extension.packageJSON;
        if (packageJSON && packageJSON.contributes && packageJSON.contributes.jsonValidation) {
            const jsonValidation = packageJSON.contributes.jsonValidation;
            if (Array.isArray(jsonValidation)) {
                jsonValidation.forEach(jv => {
                    let { fileMatch, url } = jv;
                    if (typeof fileMatch === 'string') {
                        fileMatch = [fileMatch];
                    }
                    if (Array.isArray(fileMatch) && typeof url === 'string') {
                        let uri = url;
                        if (uri[0] === '.' && uri[1] === '/') {
                            uri = requests_1.joinPath(extension.extensionUri, uri).toString();
                        }
                        fileMatch = fileMatch.map(fm => {
                            if (fm[0] === '%') {
                                fm = fm.replace(/%APP_SETTINGS_HOME%/, '/User');
                                fm = fm.replace(/%MACHINE_SETTINGS_HOME%/, '/Machine');
                                fm = fm.replace(/%APP_WORKSPACES_HOME%/, '/Workspaces');
                            }
                            else if (!fm.match(/^(\w+:\/\/|\/|!)/)) {
                                fm = '/' + fm;
                            }
                            return fm;
                        });
                        associations.push({ fileMatch, uri });
                    }
                });
            }
        }
    });
    return associations;
}
function getSettings() {
    const httpSettings = vscode_1.workspace.getConfiguration('http');
    const resultLimit = Math.trunc(Math.max(0, Number(vscode_1.workspace.getConfiguration().get(SettingIds.maxItemsComputed)))) || 5000;
    const settings = {
        http: {
            proxy: httpSettings.get('proxy'),
            proxyStrictSSL: httpSettings.get('proxyStrictSSL')
        },
        json: {
            schemas: [],
            resultLimit
        }
    };
    const schemaSettingsById = Object.create(null);
    const collectSchemaSettings = (schemaSettings, folderUri, isMultiRoot) => {
        let fileMatchPrefix = undefined;
        if (folderUri && isMultiRoot) {
            fileMatchPrefix = folderUri.toString();
            if (fileMatchPrefix[fileMatchPrefix.length - 1] === '/') {
                fileMatchPrefix = fileMatchPrefix.substr(0, fileMatchPrefix.length - 1);
            }
        }
        for (const setting of schemaSettings) {
            const url = getSchemaId(setting, folderUri);
            if (!url) {
                continue;
            }
            let schemaSetting = schemaSettingsById[url];
            if (!schemaSetting) {
                schemaSetting = schemaSettingsById[url] = { url, fileMatch: [] };
                settings.json.schemas.push(schemaSetting);
            }
            const fileMatches = setting.fileMatch;
            if (Array.isArray(fileMatches)) {
                const resultingFileMatches = schemaSetting.fileMatch || [];
                schemaSetting.fileMatch = resultingFileMatches;
                const addMatch = (pattern) => {
                    if (resultingFileMatches.indexOf(pattern) === -1) {
                        resultingFileMatches.push(pattern);
                    }
                };
                for (const fileMatch of fileMatches) {
                    if (fileMatchPrefix) {
                        if (fileMatch[0] === '/') {
                            addMatch(fileMatchPrefix + fileMatch);
                            addMatch(fileMatchPrefix + '/*' + fileMatch);
                        }
                        else {
                            addMatch(fileMatchPrefix + '/' + fileMatch);
                            addMatch(fileMatchPrefix + '/*/' + fileMatch);
                        }
                    }
                    else {
                        addMatch(fileMatch);
                    }
                }
            }
            if (setting.schema && !schemaSetting.schema) {
                schemaSetting.schema = setting.schema;
            }
        }
    };
    const folders = vscode_1.workspace.workspaceFolders;
    // merge global and folder settings. Qualify all file matches with the folder path.
    const globalSettings = vscode_1.workspace.getConfiguration('json', null).get('schemas');
    if (Array.isArray(globalSettings)) {
        if (!folders) {
            collectSchemaSettings(globalSettings);
        }
    }
    if (folders) {
        const isMultiRoot = folders.length > 1;
        for (const folder of folders) {
            const folderUri = folder.uri;
            const schemaConfigInfo = vscode_1.workspace.getConfiguration('json', folderUri).inspect('schemas');
            const folderSchemas = schemaConfigInfo.workspaceFolderValue;
            if (Array.isArray(folderSchemas)) {
                collectSchemaSettings(folderSchemas, folderUri, isMultiRoot);
            }
            if (Array.isArray(globalSettings)) {
                collectSchemaSettings(globalSettings, folderUri, isMultiRoot);
            }
        }
    }
    return settings;
}
function getSchemaId(schema, folderUri) {
    let url = schema.url;
    if (!url) {
        if (schema.schema) {
            url = schema.schema.id || `vscode://schemas/custom/${encodeURIComponent(hash_1.hash(schema.schema).toString(16))}`;
        }
    }
    else if (folderUri && (url[0] === '.' || url[0] === '/')) {
        url = requests_1.joinPath(folderUri, url).toString();
    }
    return url;
}
function isThenable(obj) {
    return obj && obj['then'];
}
function updateMarkdownString(h) {
    const n = new vscode_1.MarkdownString(h.value, true);
    n.isTrusted = h.isTrusted;
    return n;
}
function isSchemaResolveError(d) {
    return d.code === /* SchemaResolveError */ 0x300;
}
//# sourceMappingURL=jsonClient.js.map