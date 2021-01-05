"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.register = void 0;
const vscode = require("vscode");
const nls = require("vscode-nls");
const PConst = require("../protocol.const");
const typescriptService_1 = require("../typescriptService");
const api_1 = require("../utils/api");
const cancellation_1 = require("../utils/cancellation");
const codeAction_1 = require("../utils/codeAction");
const dependentRegistration_1 = require("../utils/dependentRegistration");
const modifiers_1 = require("../utils/modifiers");
const Previewer = require("../utils/previewer");
const snippetForFunctionCall_1 = require("../utils/snippetForFunctionCall");
const typeConverters = require("../utils/typeConverters");
const localize = nls.loadMessageBundle();
class MyCompletionItem extends vscode.CompletionItem {
    constructor(position, document, tsEntry, completionContext, metadata) {
        super(tsEntry.name, MyCompletionItem.convertKind(tsEntry.kind));
        this.position = position;
        this.document = document;
        this.tsEntry = tsEntry;
        this.completionContext = completionContext;
        this.metadata = metadata;
        if (tsEntry.source) {
            // De-prioritze auto-imports
            // https://github.com/microsoft/vscode/issues/40311
            this.sortText = '\uffff' + tsEntry.sortText;
            // Render "fancy" when source is a workspace path
            const qualifierCandidate = vscode.workspace.asRelativePath(tsEntry.source);
            if (qualifierCandidate !== tsEntry.source) {
                this.label2 = { name: tsEntry.name, qualifier: qualifierCandidate };
            }
        }
        else {
            this.sortText = tsEntry.sortText;
        }
        this.preselect = tsEntry.isRecommended;
        this.position = position;
        this.useCodeSnippet = completionContext.useCodeSnippetsOnMethodSuggest && (this.kind === vscode.CompletionItemKind.Function || this.kind === vscode.CompletionItemKind.Method);
        this.range = this.getRangeFromReplacementSpan(tsEntry, completionContext, position);
        this.commitCharacters = MyCompletionItem.getCommitCharacters(completionContext, tsEntry);
        this.insertText = tsEntry.insertText;
        this.filterText = this.getFilterText(completionContext.line, tsEntry.insertText);
        if (completionContext.isMemberCompletion && completionContext.dotAccessorContext) {
            this.filterText = completionContext.dotAccessorContext.text + (this.insertText || this.label);
            if (!this.range) {
                const replacementRange = this.getFuzzyWordRange();
                if (replacementRange) {
                    this.range = {
                        inserting: completionContext.dotAccessorContext.range,
                        replacing: completionContext.dotAccessorContext.range.union(replacementRange),
                    };
                }
                else {
                    this.range = completionContext.dotAccessorContext.range;
                }
                this.insertText = this.filterText;
            }
        }
        if (tsEntry.kindModifiers) {
            const kindModifiers = modifiers_1.parseKindModifier(tsEntry.kindModifiers);
            if (kindModifiers.has(PConst.KindModifiers.optional)) {
                if (!this.insertText) {
                    this.insertText = this.label;
                }
                if (!this.filterText) {
                    this.filterText = this.label;
                }
                this.label += '?';
            }
            if (kindModifiers.has(PConst.KindModifiers.depreacted)) {
                this.tags = [vscode.CompletionItemTag.Deprecated];
            }
            if (kindModifiers.has(PConst.KindModifiers.color)) {
                this.kind = vscode.CompletionItemKind.Color;
            }
            if (tsEntry.kind === PConst.Kind.script) {
                for (const extModifier of PConst.KindModifiers.fileExtensionKindModifiers) {
                    if (kindModifiers.has(extModifier)) {
                        if (tsEntry.name.toLowerCase().endsWith(extModifier)) {
                            this.detail = tsEntry.name;
                        }
                        else {
                            this.detail = tsEntry.name + extModifier;
                        }
                        break;
                    }
                }
            }
        }
        this.resolveRange();
    }
    getRangeFromReplacementSpan(tsEntry, completionContext, position) {
        if (!tsEntry.replacementSpan) {
            return;
        }
        let replaceRange = typeConverters.Range.fromTextSpan(tsEntry.replacementSpan);
        // Make sure we only replace a single line at most
        if (!replaceRange.isSingleLine) {
            replaceRange = new vscode.Range(replaceRange.start.line, replaceRange.start.character, replaceRange.start.line, completionContext.line.length);
        }
        return {
            inserting: new vscode.Range(replaceRange.start, position),
            replacing: replaceRange,
        };
    }
    getFilterText(line, insertText) {
        // Handle private field completions
        if (this.tsEntry.name.startsWith('#')) {
            const wordRange = this.completionContext.wordRange;
            const wordStart = wordRange ? line.charAt(wordRange.start.character) : undefined;
            if (insertText) {
                if (insertText.startsWith('this.#')) {
                    return wordStart === '#' ? insertText : insertText.replace(/^this\.#/, '');
                }
                else {
                    return insertText;
                }
            }
            else {
                return wordStart === '#' ? undefined : this.tsEntry.name.replace(/^#/, '');
            }
        }
        // For `this.` completions, generally don't set the filter text since we don't want them to be overly prioritized. #74164
        if (insertText === null || insertText === void 0 ? void 0 : insertText.startsWith('this.')) {
            return undefined;
        }
        // Handle the case:
        // ```
        // const xyz = { 'ab c': 1 };
        // xyz.ab|
        // ```
        // In which case we want to insert a bracket accessor but should use `.abc` as the filter text instead of
        // the bracketed insert text.
        else if (insertText === null || insertText === void 0 ? void 0 : insertText.startsWith('[')) {
            return insertText.replace(/^\[['"](.+)[['"]\]$/, '.$1');
        }
        // In all other cases, fallback to using the insertText
        return insertText;
    }
    resolveRange() {
        if (this.range) {
            return;
        }
        const replaceRange = this.getFuzzyWordRange();
        if (replaceRange) {
            this.range = {
                inserting: new vscode.Range(replaceRange.start, this.position),
                replacing: replaceRange
            };
        }
    }
    getFuzzyWordRange() {
        if (this.completionContext.useFuzzyWordRangeLogic) {
            // Try getting longer, prefix based range for completions that span words
            const text = this.completionContext.line.slice(Math.max(0, this.position.character - this.label.length), this.position.character).toLowerCase();
            const entryName = this.label.toLowerCase();
            for (let i = entryName.length; i >= 0; --i) {
                if (text.endsWith(entryName.substr(0, i)) && (!this.completionContext.wordRange || this.completionContext.wordRange.start.character > this.position.character - i)) {
                    return new vscode.Range(new vscode.Position(this.position.line, Math.max(0, this.position.character - i)), this.position);
                }
            }
        }
        return this.completionContext.wordRange;
    }
    static convertKind(kind) {
        switch (kind) {
            case PConst.Kind.primitiveType:
            case PConst.Kind.keyword:
                return vscode.CompletionItemKind.Keyword;
            case PConst.Kind.const:
            case PConst.Kind.let:
            case PConst.Kind.variable:
            case PConst.Kind.localVariable:
            case PConst.Kind.alias:
            case PConst.Kind.parameter:
                return vscode.CompletionItemKind.Variable;
            case PConst.Kind.memberVariable:
            case PConst.Kind.memberGetAccessor:
            case PConst.Kind.memberSetAccessor:
                return vscode.CompletionItemKind.Field;
            case PConst.Kind.function:
            case PConst.Kind.localFunction:
                return vscode.CompletionItemKind.Function;
            case PConst.Kind.method:
            case PConst.Kind.constructSignature:
            case PConst.Kind.callSignature:
            case PConst.Kind.indexSignature:
                return vscode.CompletionItemKind.Method;
            case PConst.Kind.enum:
                return vscode.CompletionItemKind.Enum;
            case PConst.Kind.enumMember:
                return vscode.CompletionItemKind.EnumMember;
            case PConst.Kind.module:
            case PConst.Kind.externalModuleName:
                return vscode.CompletionItemKind.Module;
            case PConst.Kind.class:
            case PConst.Kind.type:
                return vscode.CompletionItemKind.Class;
            case PConst.Kind.interface:
                return vscode.CompletionItemKind.Interface;
            case PConst.Kind.warning:
                return vscode.CompletionItemKind.Text;
            case PConst.Kind.script:
                return vscode.CompletionItemKind.File;
            case PConst.Kind.directory:
                return vscode.CompletionItemKind.Folder;
            case PConst.Kind.string:
                return vscode.CompletionItemKind.Constant;
            default:
                return vscode.CompletionItemKind.Property;
        }
    }
    static getCommitCharacters(context, entry) {
        if (context.isNewIdentifierLocation || !context.isInValidCommitCharacterContext) {
            return undefined;
        }
        const commitCharacters = [];
        switch (entry.kind) {
            case PConst.Kind.memberGetAccessor:
            case PConst.Kind.memberSetAccessor:
            case PConst.Kind.constructSignature:
            case PConst.Kind.callSignature:
            case PConst.Kind.indexSignature:
            case PConst.Kind.enum:
            case PConst.Kind.interface:
                commitCharacters.push('.', ';');
                break;
            case PConst.Kind.module:
            case PConst.Kind.alias:
            case PConst.Kind.const:
            case PConst.Kind.let:
            case PConst.Kind.variable:
            case PConst.Kind.localVariable:
            case PConst.Kind.memberVariable:
            case PConst.Kind.class:
            case PConst.Kind.function:
            case PConst.Kind.method:
            case PConst.Kind.keyword:
            case PConst.Kind.parameter:
                commitCharacters.push('.', ',', ';');
                if (context.enableCallCompletions) {
                    commitCharacters.push('(');
                }
                break;
        }
        return commitCharacters.length === 0 ? undefined : commitCharacters;
    }
}
class CompositeCommand {
    constructor() {
        this.id = CompositeCommand.ID;
    }
    execute(...commands) {
        for (const command of commands) {
            vscode.commands.executeCommand(command.command, ...(command.arguments || []));
        }
    }
}
CompositeCommand.ID = '_typescript.composite';
class CompletionAcceptedCommand {
    constructor(onCompletionAccepted, telemetryReporter) {
        this.onCompletionAccepted = onCompletionAccepted;
        this.telemetryReporter = telemetryReporter;
        this.id = CompletionAcceptedCommand.ID;
    }
    execute(item) {
        this.onCompletionAccepted(item);
        if (item instanceof MyCompletionItem) {
            /* __GDPR__
                "completions.accept" : {
                    "isPackageJsonImport" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" },
                    "${include}": [
                        "${TypeScriptCommonProperties}"
                    ]
                }
            */
            this.telemetryReporter.logTelemetry('completions.accept', {
                isPackageJsonImport: item.tsEntry.isPackageJsonImport ? 'true' : undefined,
            });
        }
    }
}
CompletionAcceptedCommand.ID = '_typescript.onCompletionAccepted';
class ApplyCompletionCodeActionCommand {
    constructor(client) {
        this.client = client;
        this.id = ApplyCompletionCodeActionCommand.ID;
    }
    async execute(_file, codeActions) {
        if (codeActions.length === 0) {
            return true;
        }
        if (codeActions.length === 1) {
            return codeAction_1.applyCodeAction(this.client, codeActions[0], cancellation_1.nulToken);
        }
        const selection = await vscode.window.showQuickPick(codeActions.map(action => ({
            label: action.description,
            description: '',
            action,
        })), {
            placeHolder: localize('selectCodeAction', 'Select code action to apply')
        });
        if (selection) {
            return codeAction_1.applyCodeAction(this.client, selection.action, cancellation_1.nulToken);
        }
        return false;
    }
}
ApplyCompletionCodeActionCommand.ID = '_typescript.applyCompletionCodeAction';
var CompletionConfiguration;
(function (CompletionConfiguration) {
    CompletionConfiguration.useCodeSnippetsOnMethodSuggest = 'suggest.completeFunctionCalls';
    CompletionConfiguration.nameSuggestions = 'suggest.names';
    CompletionConfiguration.pathSuggestions = 'suggest.paths';
    CompletionConfiguration.autoImportSuggestions = 'suggest.autoImports';
    function getConfigurationForResource(modeId, resource) {
        const config = vscode.workspace.getConfiguration(modeId, resource);
        return {
            useCodeSnippetsOnMethodSuggest: config.get(CompletionConfiguration.useCodeSnippetsOnMethodSuggest, false),
            pathSuggestions: config.get(CompletionConfiguration.pathSuggestions, true),
            autoImportSuggestions: config.get(CompletionConfiguration.autoImportSuggestions, true),
            nameSuggestions: config.get(CompletionConfiguration.nameSuggestions, true),
        };
    }
    CompletionConfiguration.getConfigurationForResource = getConfigurationForResource;
})(CompletionConfiguration || (CompletionConfiguration = {}));
class TypeScriptCompletionItemProvider {
    constructor(client, modeId, typingsStatus, fileConfigurationManager, commandManager, telemetryReporter, onCompletionAccepted) {
        this.client = client;
        this.modeId = modeId;
        this.typingsStatus = typingsStatus;
        this.fileConfigurationManager = fileConfigurationManager;
        this.telemetryReporter = telemetryReporter;
        commandManager.register(new ApplyCompletionCodeActionCommand(this.client));
        commandManager.register(new CompositeCommand());
        commandManager.register(new CompletionAcceptedCommand(onCompletionAccepted, this.telemetryReporter));
    }
    async provideCompletionItems(document, position, token, context) {
        if (this.typingsStatus.isAcquiringTypings) {
            return Promise.reject({
                label: localize({ key: 'acquiringTypingsLabel', comment: ['Typings refers to the *.d.ts typings files that power our IntelliSense. It should not be localized'] }, 'Acquiring typings...'),
                detail: localize({ key: 'acquiringTypingsDetail', comment: ['Typings refers to the *.d.ts typings files that power our IntelliSense. It should not be localized'] }, 'Acquiring typings definitions for IntelliSense.')
            });
        }
        const file = this.client.toOpenedFilePath(document);
        if (!file) {
            return undefined;
        }
        const line = document.lineAt(position.line);
        const completionConfiguration = CompletionConfiguration.getConfigurationForResource(this.modeId, document.uri);
        if (!this.shouldTrigger(context, line, position)) {
            return undefined;
        }
        const wordRange = document.getWordRangeAtPosition(position);
        await this.client.interruptGetErr(() => this.fileConfigurationManager.ensureConfigurationForDocument(document, token));
        const args = {
            ...typeConverters.Position.toFileLocationRequestArgs(file, position),
            includeExternalModuleExports: completionConfiguration.autoImportSuggestions,
            includeInsertTextCompletions: true,
            triggerCharacter: this.getTsTriggerCharacter(context),
        };
        let isNewIdentifierLocation = true;
        let isIncomplete = false;
        let isMemberCompletion = false;
        let dotAccessorContext;
        let entries;
        let metadata;
        let response;
        let duration;
        if (this.client.apiVersion.gte(api_1.default.v300)) {
            const startTime = Date.now();
            try {
                response = await this.client.interruptGetErr(() => this.client.execute('completionInfo', args, token));
            }
            finally {
                duration = Date.now() - startTime;
            }
            if (response.type !== 'response' || !response.body) {
                this.logCompletionsTelemetry(duration, response);
                return undefined;
            }
            isNewIdentifierLocation = response.body.isNewIdentifierLocation;
            isMemberCompletion = response.body.isMemberCompletion;
            if (isMemberCompletion) {
                const dotMatch = line.text.slice(0, position.character).match(/\??\.\s*$/) || undefined;
                if (dotMatch) {
                    const range = new vscode.Range(position.translate({ characterDelta: -dotMatch[0].length }), position);
                    const text = document.getText(range);
                    dotAccessorContext = { range, text };
                }
            }
            isIncomplete = response.metadata && response.metadata.isIncomplete;
            entries = response.body.entries;
            metadata = response.metadata;
        }
        else {
            const response = await this.client.interruptGetErr(() => this.client.execute('completions', args, token));
            if (response.type !== 'response' || !response.body) {
                return undefined;
            }
            entries = response.body;
            metadata = response.metadata;
        }
        const completionContext = {
            isNewIdentifierLocation,
            isMemberCompletion,
            dotAccessorContext,
            isInValidCommitCharacterContext: this.isInValidCommitCharacterContext(document, position),
            enableCallCompletions: !completionConfiguration.useCodeSnippetsOnMethodSuggest,
            wordRange,
            line: line.text,
            useCodeSnippetsOnMethodSuggest: completionConfiguration.useCodeSnippetsOnMethodSuggest,
            useFuzzyWordRangeLogic: this.client.apiVersion.lt(api_1.default.v390),
        };
        let includesPackageJsonImport = false;
        const items = [];
        for (let entry of entries) {
            if (!shouldExcludeCompletionEntry(entry, completionConfiguration)) {
                items.push(new MyCompletionItem(position, document, entry, completionContext, metadata));
                includesPackageJsonImport = !!entry.isPackageJsonImport;
            }
        }
        if (duration !== undefined) {
            this.logCompletionsTelemetry(duration, response, includesPackageJsonImport);
        }
        return new vscode.CompletionList(items, isIncomplete);
    }
    logCompletionsTelemetry(duration, response, includesPackageJsonImport) {
        var _a, _b, _c;
        /* __GDPR__
            "completions.execute" : {
                "duration" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" },
                "type" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" },
                "count" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" },
                "updateGraphDurationMs" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" },
                "createAutoImportProviderProgramDurationMs" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" },
                "includesPackageJsonImport" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" },
                "${include}": [
                    "${TypeScriptCommonProperties}"
                ]
            }
        */
        this.telemetryReporter.logTelemetry('completions.execute', {
            duration: duration,
            type: (_a = response === null || response === void 0 ? void 0 : response.type) !== null && _a !== void 0 ? _a : 'unknown',
            count: (response === null || response === void 0 ? void 0 : response.type) === 'response' && response.body ? response.body.entries.length : 0,
            updateGraphDurationMs: (response === null || response === void 0 ? void 0 : response.type) === 'response' ? (_b = response.performanceData) === null || _b === void 0 ? void 0 : _b.updateGraphDurationMs : undefined,
            createAutoImportProviderProgramDurationMs: (response === null || response === void 0 ? void 0 : response.type) === 'response' ? (_c = response.performanceData) === null || _c === void 0 ? void 0 : _c.createAutoImportProviderProgramDurationMs : undefined,
            includesPackageJsonImport: includesPackageJsonImport ? 'true' : undefined,
        });
    }
    getTsTriggerCharacter(context) {
        switch (context.triggerCharacter) {
            case '@': // Workaround for https://github.com/microsoft/TypeScript/issues/27321
                return this.client.apiVersion.gte(api_1.default.v310) && this.client.apiVersion.lt(api_1.default.v320) ? undefined : '@';
            case '#': // Workaround for https://github.com/microsoft/TypeScript/issues/36367
                return this.client.apiVersion.lt(api_1.default.v381) ? undefined : '#';
            case '.':
            case '"':
            case '\'':
            case '`':
            case '/':
            case '<':
                return context.triggerCharacter;
        }
        return undefined;
    }
    async resolveCompletionItem(item, token) {
        const filepath = this.client.toOpenedFilePath(item.document);
        if (!filepath) {
            return undefined;
        }
        const args = {
            ...typeConverters.Position.toFileLocationRequestArgs(filepath, item.position),
            entryNames: [
                item.tsEntry.source ? { name: item.tsEntry.name, source: item.tsEntry.source } : item.tsEntry.name
            ]
        };
        const response = await this.client.interruptGetErr(() => this.client.execute('completionEntryDetails', args, token));
        if (response.type !== 'response' || !response.body || !response.body.length) {
            return item;
        }
        const detail = response.body[0];
        if (!item.detail && detail.displayParts.length) {
            item.detail = Previewer.plain(detail.displayParts);
        }
        item.documentation = this.getDocumentation(detail, item);
        const codeAction = this.getCodeActions(detail, filepath);
        const commands = [{
                command: CompletionAcceptedCommand.ID,
                title: '',
                arguments: [item]
            }];
        if (codeAction.command) {
            commands.push(codeAction.command);
        }
        item.additionalTextEdits = codeAction.additionalTextEdits;
        if (item.useCodeSnippet) {
            const shouldCompleteFunction = await this.isValidFunctionCompletionContext(filepath, item.position, item.document, token);
            if (shouldCompleteFunction) {
                const { snippet, parameterCount } = snippetForFunctionCall_1.snippetForFunctionCall(item, detail.displayParts);
                item.insertText = snippet;
                if (parameterCount > 0) {
                    //Fix for https://github.com/microsoft/vscode/issues/104059
                    //Don't show parameter hints if "editor.parameterHints.enabled": false
                    if (vscode.workspace.getConfiguration('editor.parameterHints').get('enabled')) {
                        commands.push({ title: 'triggerParameterHints', command: 'editor.action.triggerParameterHints' });
                    }
                }
            }
        }
        if (commands.length) {
            if (commands.length === 1) {
                item.command = commands[0];
            }
            else {
                item.command = {
                    command: CompositeCommand.ID,
                    title: '',
                    arguments: commands
                };
            }
        }
        return item;
    }
    getCodeActions(detail, filepath) {
        if (!detail.codeActions || !detail.codeActions.length) {
            return {};
        }
        // Try to extract out the additionalTextEdits for the current file.
        // Also check if we still have to apply other workspace edits and commands
        // using a vscode command
        const additionalTextEdits = [];
        let hasReaminingCommandsOrEdits = false;
        for (const tsAction of detail.codeActions) {
            if (tsAction.commands) {
                hasReaminingCommandsOrEdits = true;
            }
            // Apply all edits in the current file using `additionalTextEdits`
            if (tsAction.changes) {
                for (const change of tsAction.changes) {
                    if (change.fileName === filepath) {
                        additionalTextEdits.push(...change.textChanges.map(typeConverters.TextEdit.fromCodeEdit));
                    }
                    else {
                        hasReaminingCommandsOrEdits = true;
                    }
                }
            }
        }
        let command = undefined;
        if (hasReaminingCommandsOrEdits) {
            // Create command that applies all edits not in the current file.
            command = {
                title: '',
                command: ApplyCompletionCodeActionCommand.ID,
                arguments: [filepath, detail.codeActions.map((x) => ({
                        commands: x.commands,
                        description: x.description,
                        changes: x.changes.filter(x => x.fileName !== filepath)
                    }))]
            };
        }
        return {
            command,
            additionalTextEdits: additionalTextEdits.length ? additionalTextEdits : undefined
        };
    }
    isInValidCommitCharacterContext(document, position) {
        if (this.client.apiVersion.lt(api_1.default.v320)) {
            // Workaround for https://github.com/microsoft/TypeScript/issues/27742
            // Only enable dot completions when previous character not a dot preceded by whitespace.
            // Prevents incorrectly completing while typing spread operators.
            if (position.character > 1) {
                const preText = document.getText(new vscode.Range(position.line, 0, position.line, position.character));
                return preText.match(/(\s|^)\.$/ig) === null;
            }
        }
        return true;
    }
    shouldTrigger(context, line, position) {
        if (context.triggerCharacter && this.client.apiVersion.lt(api_1.default.v290)) {
            if ((context.triggerCharacter === '"' || context.triggerCharacter === '\'')) {
                // make sure we are in something that looks like the start of an import
                const pre = line.text.slice(0, position.character);
                if (!pre.match(/\b(from|import)\s*["']$/) && !pre.match(/\b(import|require)\(['"]$/)) {
                    return false;
                }
            }
            if (context.triggerCharacter === '/') {
                // make sure we are in something that looks like an import path
                const pre = line.text.slice(0, position.character);
                if (!pre.match(/\b(from|import)\s*["'][^'"]*$/) && !pre.match(/\b(import|require)\(['"][^'"]*$/)) {
                    return false;
                }
            }
            if (context.triggerCharacter === '@') {
                // make sure we are in something that looks like the start of a jsdoc comment
                const pre = line.text.slice(0, position.character);
                if (!pre.match(/^\s*\*[ ]?@/) && !pre.match(/\/\*\*+[ ]?@/)) {
                    return false;
                }
            }
            if (context.triggerCharacter === '<') {
                return false;
            }
        }
        return true;
    }
    getDocumentation(detail, item) {
        const documentation = new vscode.MarkdownString();
        if (detail.source) {
            const importPath = `'${Previewer.plain(detail.source)}'`;
            const autoImportLabel = localize('autoImportLabel', 'Auto import from {0}', importPath);
            item.detail = `${autoImportLabel}\n${item.detail}`;
        }
        Previewer.addMarkdownDocumentation(documentation, detail.documentation, detail.tags);
        return documentation.value.length ? documentation : undefined;
    }
    async isValidFunctionCompletionContext(filepath, position, document, token) {
        // Workaround for https://github.com/microsoft/TypeScript/issues/12677
        // Don't complete function calls inside of destructive assignments or imports
        try {
            const args = typeConverters.Position.toFileLocationRequestArgs(filepath, position);
            const response = await this.client.execute('quickinfo', args, token);
            if (response.type === 'response' && response.body) {
                switch (response.body.kind) {
                    case 'var':
                    case 'let':
                    case 'const':
                    case 'alias':
                        return false;
                }
            }
        }
        catch (_a) {
            // Noop
        }
        // Don't complete function call if there is already something that looks like a function call
        // https://github.com/microsoft/vscode/issues/18131
        const after = document.lineAt(position.line).text.slice(position.character);
        return after.match(/^[a-z_$0-9]*\s*\(/gi) === null;
    }
}
TypeScriptCompletionItemProvider.triggerCharacters = ['.', '"', '\'', '`', '/', '@', '<', '#'];
function shouldExcludeCompletionEntry(element, completionConfiguration) {
    return ((!completionConfiguration.nameSuggestions && element.kind === PConst.Kind.warning)
        || (!completionConfiguration.pathSuggestions &&
            (element.kind === PConst.Kind.directory || element.kind === PConst.Kind.script || element.kind === PConst.Kind.externalModuleName))
        || (!completionConfiguration.autoImportSuggestions && element.hasAction));
}
function register(selector, modeId, client, typingsStatus, fileConfigurationManager, commandManager, telemetryReporter, onCompletionAccepted) {
    return dependentRegistration_1.conditionalRegistration([
        dependentRegistration_1.requireConfiguration(modeId, 'suggest.enabled'),
        dependentRegistration_1.requireSomeCapability(client, typescriptService_1.ClientCapability.EnhancedSyntax, typescriptService_1.ClientCapability.Semantic),
    ], () => {
        return vscode.languages.registerCompletionItemProvider(selector.syntax, new TypeScriptCompletionItemProvider(client, modeId, typingsStatus, fileConfigurationManager, commandManager, telemetryReporter, onCompletionAccepted), ...TypeScriptCompletionItemProvider.triggerCharacters);
    });
}
exports.register = register;
//# sourceMappingURL=completions.js.map