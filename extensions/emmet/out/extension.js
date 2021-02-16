"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.deactivate = exports.activate = void 0;
const vscode = require("vscode");
const defaultCompletionProvider_1 = require("./defaultCompletionProvider");
const abbreviationActions_1 = require("./abbreviationActions");
const removeTag_1 = require("./removeTag");
const updateTag_1 = require("./updateTag");
const matchTag_1 = require("./matchTag");
const balance_1 = require("./balance");
const splitJoinTag_1 = require("./splitJoinTag");
const mergeLines_1 = require("./mergeLines");
const toggleComment_1 = require("./toggleComment");
const editPoint_1 = require("./editPoint");
const selectItem_1 = require("./selectItem");
const evaluateMathExpression_1 = require("./evaluateMathExpression");
const incrementDecrement_1 = require("./incrementDecrement");
const util_1 = require("./util");
const updateImageSize_1 = require("./updateImageSize");
const reflectCssValue_1 = require("./reflectCssValue");
function activate(context) {
    registerCompletionProviders(context);
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.wrapWithAbbreviation', (args) => {
        abbreviationActions_1.wrapWithAbbreviation(args);
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.wrapIndividualLinesWithAbbreviation', (args) => {
        abbreviationActions_1.wrapIndividualLinesWithAbbreviation(args);
    }));
    context.subscriptions.push(vscode.commands.registerCommand('emmet.expandAbbreviation', (args) => {
        abbreviationActions_1.expandEmmetAbbreviation(args);
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.removeTag', () => {
        return removeTag_1.removeTag();
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.updateTag', (inputTag) => {
        if (inputTag && typeof inputTag === 'string') {
            return updateTag_1.updateTag(inputTag);
        }
        return vscode.window.showInputBox({ prompt: 'Enter Tag' }).then(tagName => {
            if (tagName) {
                const update = updateTag_1.updateTag(tagName);
                return update ? update : false;
            }
            return false;
        });
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.matchTag', () => {
        matchTag_1.matchTag();
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.balanceOut', () => {
        balance_1.balanceOut();
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.balanceIn', () => {
        balance_1.balanceIn();
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.splitJoinTag', () => {
        return splitJoinTag_1.splitJoinTag();
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.mergeLines', () => {
        mergeLines_1.mergeLines();
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.toggleComment', () => {
        toggleComment_1.toggleComment();
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.nextEditPoint', () => {
        editPoint_1.fetchEditPoint('next');
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.prevEditPoint', () => {
        editPoint_1.fetchEditPoint('prev');
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.selectNextItem', () => {
        selectItem_1.fetchSelectItem('next');
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.selectPrevItem', () => {
        selectItem_1.fetchSelectItem('prev');
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.evaluateMathExpression', () => {
        evaluateMathExpression_1.evaluateMathExpression();
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.incrementNumberByOneTenth', () => {
        return incrementDecrement_1.incrementDecrement(0.1);
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.incrementNumberByOne', () => {
        return incrementDecrement_1.incrementDecrement(1);
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.incrementNumberByTen', () => {
        return incrementDecrement_1.incrementDecrement(10);
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.decrementNumberByOneTenth', () => {
        return incrementDecrement_1.incrementDecrement(-0.1);
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.decrementNumberByOne', () => {
        return incrementDecrement_1.incrementDecrement(-1);
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.decrementNumberByTen', () => {
        return incrementDecrement_1.incrementDecrement(-10);
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.updateImageSize', () => {
        return updateImageSize_1.updateImageSize();
    }));
    context.subscriptions.push(vscode.commands.registerCommand('editor.emmet.action.reflectCSSValue', () => {
        return reflectCssValue_1.reflectCssValue();
    }));
    util_1.updateEmmetExtensionsPath();
    context.subscriptions.push(vscode.workspace.onDidChangeConfiguration((e) => {
        if (e.affectsConfiguration('emmet.includeLanguages')) {
            registerCompletionProviders(context);
        }
        if (e.affectsConfiguration('emmet.extensionsPath')) {
            util_1.updateEmmetExtensionsPath();
        }
    }));
}
exports.activate = activate;
/**
 * Holds any registered completion providers by their language strings
 */
const languageMappingForCompletionProviders = new Map();
const completionProvidersMapping = new Map();
function registerCompletionProviders(context) {
    let completionProvider = new defaultCompletionProvider_1.DefaultCompletionItemProvider();
    let includedLanguages = util_1.getMappingForIncludedLanguages();
    Object.keys(includedLanguages).forEach(language => {
        if (languageMappingForCompletionProviders.has(language) && languageMappingForCompletionProviders.get(language) === includedLanguages[language]) {
            return;
        }
        if (languageMappingForCompletionProviders.has(language)) {
            const mapping = completionProvidersMapping.get(language);
            if (mapping) {
                mapping.dispose();
            }
            languageMappingForCompletionProviders.delete(language);
            completionProvidersMapping.delete(language);
        }
        const provider = vscode.languages.registerCompletionItemProvider({ language, scheme: '*' }, completionProvider, ...util_1.LANGUAGE_MODES[includedLanguages[language]]);
        context.subscriptions.push(provider);
        languageMappingForCompletionProviders.set(language, includedLanguages[language]);
        completionProvidersMapping.set(language, provider);
    });
    Object.keys(util_1.LANGUAGE_MODES).forEach(language => {
        if (!languageMappingForCompletionProviders.has(language)) {
            const provider = vscode.languages.registerCompletionItemProvider({ language, scheme: '*' }, completionProvider, ...util_1.LANGUAGE_MODES[language]);
            context.subscriptions.push(provider);
            languageMappingForCompletionProviders.set(language, language);
            completionProvidersMapping.set(language, provider);
        }
    });
}
function deactivate() {
}
exports.deactivate = deactivate;
//# sourceMappingURL=extension.js.map