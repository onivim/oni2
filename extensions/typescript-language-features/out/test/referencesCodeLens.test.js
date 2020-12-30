"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const assert = require("assert");
require("mocha");
const vscode = require("vscode");
const dispose_1 = require("../utils/dispose");
const testUtils_1 = require("./testUtils");
async function updateConfig(newConfig) {
    const oldConfig = {};
    const config = vscode.workspace.getConfiguration(undefined);
    for (const configKey of Object.keys(newConfig)) {
        oldConfig[configKey] = config.get(configKey);
        await new Promise((resolve, reject) => config.update(configKey, newConfig[configKey], vscode.ConfigurationTarget.Global)
            .then(() => resolve(), reject));
    }
    return oldConfig;
}
var Config;
(function (Config) {
    Config.referencesCodeLens = 'typescript.referencesCodeLens.enabled';
})(Config || (Config = {}));
suite('TypeScript References', () => {
    const configDefaults = Object.freeze({
        [Config.referencesCodeLens]: true,
    });
    const _disposables = [];
    let oldConfig = {};
    setup(async () => {
        await testUtils_1.wait(100);
        // Save off config and apply defaults
        oldConfig = await updateConfig(configDefaults);
    });
    teardown(async () => {
        dispose_1.disposeAll(_disposables);
        // Restore config
        await updateConfig(oldConfig);
        return vscode.commands.executeCommand('workbench.action.closeAllEditors');
    });
    test('Should show on basic class', async () => {
        const testDocumentUri = vscode.Uri.parse('untitled:test1.ts');
        await testUtils_1.createTestEditor(testDocumentUri, `class Foo {}`);
        const codeLenses = await getCodeLenses(testDocumentUri);
        assert.strictEqual(codeLenses === null || codeLenses === void 0 ? void 0 : codeLenses.length, 1);
        assert.strictEqual(codeLenses === null || codeLenses === void 0 ? void 0 : codeLenses[0].range.start.line, 0);
    });
    test('Should show on basic class properties', async () => {
        const testDocumentUri = vscode.Uri.parse('untitled:test2.ts');
        await testUtils_1.createTestEditor(testDocumentUri, `class Foo {`, `	prop: number;`, `	meth(): void {}`, `}`);
        const codeLenses = await getCodeLenses(testDocumentUri);
        assert.strictEqual(codeLenses === null || codeLenses === void 0 ? void 0 : codeLenses.length, 3);
        assert.strictEqual(codeLenses === null || codeLenses === void 0 ? void 0 : codeLenses[0].range.start.line, 0);
        assert.strictEqual(codeLenses === null || codeLenses === void 0 ? void 0 : codeLenses[1].range.start.line, 1);
        assert.strictEqual(codeLenses === null || codeLenses === void 0 ? void 0 : codeLenses[2].range.start.line, 2);
    });
    test('Should not show on const property', async () => {
        const testDocumentUri = vscode.Uri.parse('untitled:test3.ts');
        await testUtils_1.createTestEditor(testDocumentUri, `const foo = {`, `	prop: 1;`, `	meth(): void {}`, `}`);
        const codeLenses = await getCodeLenses(testDocumentUri);
        assert.strictEqual(codeLenses === null || codeLenses === void 0 ? void 0 : codeLenses.length, 0);
    });
    test.skip('Should not show duplicate references on ES5 class (https://github.com/microsoft/vscode/issues/90396)', async () => {
        const testDocumentUri = vscode.Uri.parse('untitled:test3.js');
        await testUtils_1.createTestEditor(testDocumentUri, `function A() {`, `    console.log("hi");`, `}`, `A.x = {};`);
        await testUtils_1.wait(500);
        const codeLenses = await getCodeLenses(testDocumentUri);
        assert.strictEqual(codeLenses === null || codeLenses === void 0 ? void 0 : codeLenses.length, 1);
    });
});
function getCodeLenses(document) {
    return vscode.commands.executeCommand('vscode.executeCodeLensProvider', document, 100);
}
//# sourceMappingURL=referencesCodeLens.test.js.map