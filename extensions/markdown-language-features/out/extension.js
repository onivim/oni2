"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = void 0;
const vscode = require("vscode");
const commandManager_1 = require("./commandManager");
const commands = require("./commands/index");
const documentLinkProvider_1 = require("./features/documentLinkProvider");
const documentSymbolProvider_1 = require("./features/documentSymbolProvider");
const foldingProvider_1 = require("./features/foldingProvider");
const smartSelect_1 = require("./features/smartSelect");
const previewContentProvider_1 = require("./features/previewContentProvider");
const previewManager_1 = require("./features/previewManager");
const workspaceSymbolProvider_1 = require("./features/workspaceSymbolProvider");
const logger_1 = require("./logger");
const markdownEngine_1 = require("./markdownEngine");
const markdownExtensions_1 = require("./markdownExtensions");
const security_1 = require("./security");
const slugify_1 = require("./slugify");
const telemetryReporter_1 = require("./telemetryReporter");
function activate(context) {
    const telemetryReporter = telemetryReporter_1.loadDefaultTelemetryReporter();
    context.subscriptions.push(telemetryReporter);
    const contributions = markdownExtensions_1.getMarkdownExtensionContributions(context);
    context.subscriptions.push(contributions);
    const cspArbiter = new security_1.ExtensionContentSecurityPolicyArbiter(context.globalState, context.workspaceState);
    const engine = new markdownEngine_1.MarkdownEngine(contributions, slugify_1.githubSlugifier);
    const logger = new logger_1.Logger();
    const contentProvider = new previewContentProvider_1.MarkdownContentProvider(engine, context, cspArbiter, contributions, logger);
    const symbolProvider = new documentSymbolProvider_1.default(engine);
    const previewManager = new previewManager_1.MarkdownPreviewManager(contentProvider, logger, contributions, engine);
    context.subscriptions.push(previewManager);
    context.subscriptions.push(registerMarkdownLanguageFeatures(symbolProvider, engine));
    context.subscriptions.push(registerMarkdownCommands(previewManager, telemetryReporter, cspArbiter, engine));
    context.subscriptions.push(vscode.workspace.onDidChangeConfiguration(() => {
        logger.updateConfiguration();
        previewManager.updateConfiguration();
    }));
}
exports.activate = activate;
function registerMarkdownLanguageFeatures(symbolProvider, engine) {
    const selector = { language: 'markdown', scheme: '*' };
    const charPattern = '(\\p{Alphabetic}|\\p{Number}|\\p{Nonspacing_Mark})';
    return vscode.Disposable.from(vscode.languages.setLanguageConfiguration('markdown', {
        wordPattern: new RegExp(`${charPattern}((${charPattern}|[_])?${charPattern})*`, 'ug'),
    }), vscode.languages.registerDocumentSymbolProvider(selector, symbolProvider), vscode.languages.registerDocumentLinkProvider(selector, new documentLinkProvider_1.default()), vscode.languages.registerFoldingRangeProvider(selector, new foldingProvider_1.default(engine)), vscode.languages.registerSelectionRangeProvider(selector, new smartSelect_1.default(engine)), vscode.languages.registerWorkspaceSymbolProvider(new workspaceSymbolProvider_1.default(symbolProvider)));
}
function registerMarkdownCommands(previewManager, telemetryReporter, cspArbiter, engine) {
    const previewSecuritySelector = new security_1.PreviewSecuritySelector(cspArbiter, previewManager);
    const commandManager = new commandManager_1.CommandManager();
    commandManager.register(new commands.ShowPreviewCommand(previewManager, telemetryReporter));
    commandManager.register(new commands.ShowPreviewToSideCommand(previewManager, telemetryReporter));
    commandManager.register(new commands.ShowLockedPreviewToSideCommand(previewManager, telemetryReporter));
    commandManager.register(new commands.ShowSourceCommand(previewManager));
    commandManager.register(new commands.RefreshPreviewCommand(previewManager, engine));
    commandManager.register(new commands.MoveCursorToPositionCommand());
    commandManager.register(new commands.ShowPreviewSecuritySelectorCommand(previewSecuritySelector, previewManager));
    commandManager.register(new commands.OpenDocumentLinkCommand(engine));
    commandManager.register(new commands.ToggleLockCommand(previewManager));
    commandManager.register(new commands.RenderDocument(engine));
    return commandManager;
}
//# sourceMappingURL=extension.js.map