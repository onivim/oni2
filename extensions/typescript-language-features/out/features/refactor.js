"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const vscode = require("vscode");
const api_1 = require("../utils/api");
const dependentRegistration_1 = require("../utils/dependentRegistration");
const typeConverters = require("../utils/typeConverters");
const cancellation_1 = require("../utils/cancellation");
class ApplyRefactoringCommand {
    constructor(client, telemetryReporter) {
        this.client = client;
        this.telemetryReporter = telemetryReporter;
        this.id = ApplyRefactoringCommand.ID;
    }
    async execute(document, file, refactor, action, range) {
        /* __GDPR__
            "refactor.execute" : {
                "action" : { "classification": "PublicNonPersonalData", "purpose": "FeatureInsight" },
                "${include}": [
                    "${TypeScriptCommonProperties}"
                ]
            }
        */
        this.telemetryReporter.logTelemetry('refactor.execute', {
            action: action,
        });
        const args = Object.assign({}, typeConverters.Range.toFileRangeRequestArgs(file, range), { refactor,
            action });
        const response = await this.client.execute('getEditsForRefactor', args, cancellation_1.nulToken);
        if (response.type !== 'response' || !response.body || !response.body.edits.length) {
            return false;
        }
        const workspaceEdit = await this.toWorkspaceEdit(response.body);
        if (!(await vscode.workspace.applyEdit(workspaceEdit))) {
            return false;
        }
        const renameLocation = response.body.renameLocation;
        if (renameLocation) {
            await vscode.commands.executeCommand('editor.action.rename', [
                document.uri,
                typeConverters.Position.fromLocation(renameLocation)
            ]);
        }
        return true;
    }
    async toWorkspaceEdit(body) {
        const workspaceEdit = new vscode.WorkspaceEdit();
        for (const edit of body.edits) {
            workspaceEdit.createFile(this.client.toResource(edit.fileName), { ignoreIfExists: true });
        }
        typeConverters.WorkspaceEdit.withFileCodeEdits(workspaceEdit, this.client, body.edits);
        return workspaceEdit;
    }
}
ApplyRefactoringCommand.ID = '_typescript.applyRefactoring';
class SelectRefactorCommand {
    constructor(doRefactoring) {
        this.doRefactoring = doRefactoring;
        this.id = SelectRefactorCommand.ID;
    }
    async execute(document, file, info, range) {
        const selected = await vscode.window.showQuickPick(info.actions.map((action) => ({
            label: action.name,
            description: action.description,
        })));
        if (!selected) {
            return false;
        }
        return this.doRefactoring.execute(document, file, info.name, selected.label, range);
    }
}
SelectRefactorCommand.ID = '_typescript.selectRefactoring';
class TypeScriptRefactorProvider {
    constructor(client, formattingOptionsManager, commandManager, telemetryReporter) {
        this.client = client;
        this.formattingOptionsManager = formattingOptionsManager;
        const doRefactoringCommand = commandManager.register(new ApplyRefactoringCommand(this.client, telemetryReporter));
        commandManager.register(new SelectRefactorCommand(doRefactoringCommand));
    }
    async provideCodeActions(document, rangeOrSelection, context, token) {
        if (!this.shouldTrigger(rangeOrSelection, context)) {
            return undefined;
        }
        const file = this.client.toOpenedFilePath(document);
        if (!file) {
            return undefined;
        }
        const args = typeConverters.Range.toFileRangeRequestArgs(file, rangeOrSelection);
        const response = await this.client.interruptGetErr(() => {
            this.formattingOptionsManager.ensureConfigurationForDocument(document, token);
            return this.client.execute('getApplicableRefactors', args, token);
        });
        if (response.type !== 'response' || !response.body) {
            return undefined;
        }
        return this.convertApplicableRefactors(response.body, document, file, rangeOrSelection);
    }
    convertApplicableRefactors(body, document, file, rangeOrSelection) {
        const actions = [];
        for (const info of body) {
            if (info.inlineable === false) {
                const codeAction = new vscode.CodeAction(info.description, vscode.CodeActionKind.Refactor);
                codeAction.command = {
                    title: info.description,
                    command: SelectRefactorCommand.ID,
                    arguments: [document, file, info, rangeOrSelection]
                };
                actions.push(codeAction);
            }
            else {
                for (const action of info.actions) {
                    actions.push(this.refactorActionToCodeAction(action, document, file, info, rangeOrSelection));
                }
            }
        }
        return actions;
    }
    refactorActionToCodeAction(action, document, file, info, rangeOrSelection) {
        const codeAction = new vscode.CodeAction(action.description, TypeScriptRefactorProvider.getKind(action));
        codeAction.command = {
            title: action.description,
            command: ApplyRefactoringCommand.ID,
            arguments: [document, file, info.name, action.name, rangeOrSelection],
        };
        codeAction.isPreferred = TypeScriptRefactorProvider.isPreferred(action);
        return codeAction;
    }
    shouldTrigger(rangeOrSelection, context) {
        if (context.only && !vscode.CodeActionKind.Refactor.contains(context.only)) {
            return false;
        }
        return rangeOrSelection instanceof vscode.Selection;
    }
    static getKind(refactor) {
        if (refactor.name.startsWith('function_')) {
            return TypeScriptRefactorProvider.extractFunctionKind;
        }
        else if (refactor.name.startsWith('constant_')) {
            return TypeScriptRefactorProvider.extractConstantKind;
        }
        else if (refactor.name.startsWith('Move')) {
            return TypeScriptRefactorProvider.moveKind;
        }
        return vscode.CodeActionKind.Refactor;
    }
    static isPreferred(action) {
        if (action.name.startsWith('constant_')) {
            return action.name.endsWith('scope_0');
        }
        return false;
    }
}
TypeScriptRefactorProvider.extractFunctionKind = vscode.CodeActionKind.RefactorExtract.append('function');
TypeScriptRefactorProvider.extractConstantKind = vscode.CodeActionKind.RefactorExtract.append('constant');
TypeScriptRefactorProvider.moveKind = vscode.CodeActionKind.Refactor.append('move');
TypeScriptRefactorProvider.metadata = {
    providedCodeActionKinds: [vscode.CodeActionKind.Refactor],
};
function register(selector, client, formattingOptionsManager, commandManager, telemetryReporter) {
    return new dependentRegistration_1.VersionDependentRegistration(client, api_1.default.v240, () => {
        return vscode.languages.registerCodeActionsProvider(selector, new TypeScriptRefactorProvider(client, formattingOptionsManager, commandManager, telemetryReporter), TypeScriptRefactorProvider.metadata);
    });
}
exports.register = register;
//# sourceMappingURL=refactor.js.map