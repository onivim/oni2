"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.register = exports.OrganizeImportsCodeActionProvider = void 0;
const vscode = require("vscode");
const nls = require("vscode-nls");
const typescriptService_1 = require("../typescriptService");
const api_1 = require("../utils/api");
const cancellation_1 = require("../utils/cancellation");
const dependentRegistration_1 = require("../utils/dependentRegistration");
const typeconverts = require("../utils/typeConverters");
const localize = nls.loadMessageBundle();
class OrganizeImportsCommand {
    constructor(client, telemetryReporter) {
        this.client = client;
        this.telemetryReporter = telemetryReporter;
        this.id = OrganizeImportsCommand.Id;
    }
    async execute(file) {
        /* __GDPR__
            "organizeImports.execute" : {
                "${include}": [
                    "${TypeScriptCommonProperties}"
                ]
            }
        */
        this.telemetryReporter.logTelemetry('organizeImports.execute', {});
        const args = {
            scope: {
                type: 'file',
                args: {
                    file
                }
            }
        };
        const response = await this.client.interruptGetErr(() => this.client.execute('organizeImports', args, cancellation_1.nulToken));
        if (response.type !== 'response' || !response.body) {
            return false;
        }
        const edits = typeconverts.WorkspaceEdit.fromFileCodeEdits(this.client, response.body);
        return vscode.workspace.applyEdit(edits);
    }
}
OrganizeImportsCommand.Id = '_typescript.organizeImports';
class OrganizeImportsCodeActionProvider {
    constructor(client, commandManager, fileConfigManager, telemetryReporter) {
        this.client = client;
        this.fileConfigManager = fileConfigManager;
        this.metadata = {
            providedCodeActionKinds: [vscode.CodeActionKind.SourceOrganizeImports]
        };
        commandManager.register(new OrganizeImportsCommand(client, telemetryReporter));
    }
    provideCodeActions(document, _range, context, token) {
        const file = this.client.toOpenedFilePath(document);
        if (!file) {
            return [];
        }
        if (!context.only || !context.only.contains(vscode.CodeActionKind.SourceOrganizeImports)) {
            return [];
        }
        this.fileConfigManager.ensureConfigurationForDocument(document, token);
        const action = new vscode.CodeAction(localize('organizeImportsAction.title', "Organize Imports"), vscode.CodeActionKind.SourceOrganizeImports);
        action.command = { title: '', command: OrganizeImportsCommand.Id, arguments: [file] };
        return [action];
    }
}
exports.OrganizeImportsCodeActionProvider = OrganizeImportsCodeActionProvider;
OrganizeImportsCodeActionProvider.minVersion = api_1.default.v280;
function register(selector, client, commandManager, fileConfigurationManager, telemetryReporter) {
    return dependentRegistration_1.conditionalRegistration([
        dependentRegistration_1.requireMinVersion(client, OrganizeImportsCodeActionProvider.minVersion),
        dependentRegistration_1.requireSomeCapability(client, typescriptService_1.ClientCapability.Semantic),
    ], () => {
        const organizeImportsProvider = new OrganizeImportsCodeActionProvider(client, commandManager, fileConfigurationManager, telemetryReporter);
        return vscode.languages.registerCodeActionsProvider(selector.semantic, organizeImportsProvider, organizeImportsProvider.metadata);
    });
}
exports.register = register;
//# sourceMappingURL=organizeImports.js.map