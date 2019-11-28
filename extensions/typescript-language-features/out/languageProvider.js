"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
Object.defineProperty(exports, "__esModule", { value: true });
const path_1 = require("path");
const vscode = require("vscode");
const cachedResponse_1 = require("./tsServer/cachedResponse");
const dispose_1 = require("./utils/dispose");
const fileSchemes = require("./utils/fileSchemes");
const memoize_1 = require("./utils/memoize");
const validateSetting = 'validate.enable';
const suggestionSetting = 'suggestionActions.enabled';
class LanguageProvider extends dispose_1.Disposable {
    constructor(client, description, commandManager, telemetryReporter, typingsStatus, fileConfigurationManager, onCompletionAccepted) {
        super();
        this.client = client;
        this.description = description;
        this.commandManager = commandManager;
        this.telemetryReporter = telemetryReporter;
        this.typingsStatus = typingsStatus;
        this.fileConfigurationManager = fileConfigurationManager;
        this.onCompletionAccepted = onCompletionAccepted;
        vscode.workspace.onDidChangeConfiguration(this.configurationChanged, this, this._disposables);
        this.configurationChanged();
        client.onReady(() => this.registerProviders());
    }
    get documentSelector() {
        const documentSelector = [];
        for (const language of this.description.modeIds) {
            for (const scheme of fileSchemes.supportedSchemes) {
                documentSelector.push({ language, scheme });
            }
        }
        return documentSelector;
    }
    async registerProviders() {
        const selector = this.documentSelector;
        const cachedResponse = new cachedResponse_1.CachedResponse();
        this._register((await Promise.resolve().then(() => require('./features/completions'))).register(selector, this.description.id, this.client, this.typingsStatus, this.fileConfigurationManager, this.commandManager, this.onCompletionAccepted));
        this._register((await Promise.resolve().then(() => require('./features/definitions'))).register(selector, this.client));
        this._register((await Promise.resolve().then(() => require('./features/directiveCommentCompletions'))).register(selector, this.client));
        this._register((await Promise.resolve().then(() => require('./features/documentHighlight'))).register(selector, this.client));
        this._register((await Promise.resolve().then(() => require('./features/documentSymbol'))).register(selector, this.client, cachedResponse));
        this._register((await Promise.resolve().then(() => require('./features/folding'))).register(selector, this.client));
        this._register((await Promise.resolve().then(() => require('./features/formatting'))).register(selector, this.description.id, this.client, this.fileConfigurationManager));
        this._register((await Promise.resolve().then(() => require('./features/hover'))).register(selector, this.client));
        this._register((await Promise.resolve().then(() => require('./features/implementations'))).register(selector, this.client));
        this._register((await Promise.resolve().then(() => require('./features/implementationsCodeLens'))).register(selector, this.description.id, this.client, cachedResponse));
        this._register((await Promise.resolve().then(() => require('./features/jsDocCompletions'))).register(selector, this.description.id, this.client));
        this._register((await Promise.resolve().then(() => require('./features/organizeImports'))).register(selector, this.client, this.commandManager, this.fileConfigurationManager, this.telemetryReporter));
        this._register((await Promise.resolve().then(() => require('./features/quickFix'))).register(selector, this.client, this.fileConfigurationManager, this.commandManager, this.client.diagnosticsManager, this.telemetryReporter));
        this._register((await Promise.resolve().then(() => require('./features/fixAll'))).register(selector, this.client, this.fileConfigurationManager, this.client.diagnosticsManager));
        this._register((await Promise.resolve().then(() => require('./features/refactor'))).register(selector, this.client, this.fileConfigurationManager, this.commandManager, this.telemetryReporter));
        this._register((await Promise.resolve().then(() => require('./features/references'))).register(selector, this.client));
        this._register((await Promise.resolve().then(() => require('./features/referencesCodeLens'))).register(selector, this.description.id, this.client, cachedResponse));
        this._register((await Promise.resolve().then(() => require('./features/rename'))).register(selector, this.client, this.fileConfigurationManager));
        this._register((await Promise.resolve().then(() => require('./features/signatureHelp'))).register(selector, this.client));
        this._register((await Promise.resolve().then(() => require('./features/tagClosing'))).register(selector, this.description.id, this.client));
        this._register((await Promise.resolve().then(() => require('./features/typeDefinitions'))).register(selector, this.client));
    }
    configurationChanged() {
        const config = vscode.workspace.getConfiguration(this.id, null);
        this.updateValidate(config.get(validateSetting, true));
        this.updateSuggestionDiagnostics(config.get(suggestionSetting, true));
    }
    handles(resource, doc) {
        if (doc && this.description.modeIds.indexOf(doc.languageId) >= 0) {
            return true;
        }
        const base = path_1.basename(resource.fsPath);
        return !!base && (!!this.description.configFilePattern && this.description.configFilePattern.test(base));
    }
    get id() {
        return this.description.id;
    }
    get diagnosticSource() {
        return this.description.diagnosticSource;
    }
    updateValidate(value) {
        this.client.diagnosticsManager.setValidate(this._diagnosticLanguage, value);
    }
    updateSuggestionDiagnostics(value) {
        this.client.diagnosticsManager.setEnableSuggestions(this._diagnosticLanguage, value);
    }
    reInitialize() {
        this.client.diagnosticsManager.reInitialize();
    }
    triggerAllDiagnostics() {
        this.client.bufferSyncSupport.requestAllDiagnostics();
    }
    diagnosticsReceived(diagnosticsKind, file, diagnostics) {
        const config = vscode.workspace.getConfiguration(this.id, file);
        const reportUnnecessary = config.get('showUnused', true);
        this.client.diagnosticsManager.updateDiagnostics(file, this._diagnosticLanguage, diagnosticsKind, diagnostics.filter(diag => {
            if (!reportUnnecessary) {
                diag.tags = undefined;
                if (diag.reportUnnecessary && diag.severity === vscode.DiagnosticSeverity.Hint) {
                    return false;
                }
            }
            return true;
        }));
    }
    configFileDiagnosticsReceived(file, diagnostics) {
        this.client.diagnosticsManager.configFileDiagnosticsReceived(file, diagnostics);
    }
    get _diagnosticLanguage() {
        return this.description.diagnosticLanguage;
    }
}
__decorate([
    memoize_1.memoize
], LanguageProvider.prototype, "documentSelector", null);
exports.default = LanguageProvider;
//# sourceMappingURL=languageProvider.js.map