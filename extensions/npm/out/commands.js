"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.selectAndRunScriptFromFolder = exports.runSelectedScript = void 0;
const nls = require("vscode-nls");
const vscode = require("vscode");
const tasks_1 = require("./tasks");
const localize = nls.loadMessageBundle();
function runSelectedScript(context) {
    let editor = vscode.window.activeTextEditor;
    if (!editor) {
        return;
    }
    let document = editor.document;
    let contents = document.getText();
    let selection = editor.selection;
    let offset = document.offsetAt(selection.anchor);
    let script = tasks_1.findScriptAtPosition(contents, offset);
    if (script) {
        tasks_1.runScript(context, script, document);
    }
    else {
        let message = localize('noScriptFound', 'Could not find a valid npm script at the selection.');
        vscode.window.showErrorMessage(message);
    }
}
exports.runSelectedScript = runSelectedScript;
async function selectAndRunScriptFromFolder(context, selectedFolder) {
    let taskList = await tasks_1.detectNpmScriptsForFolder(context, selectedFolder);
    if (taskList && taskList.length > 0) {
        const quickPick = vscode.window.createQuickPick();
        quickPick.title = 'Run NPM script in Folder';
        quickPick.placeholder = 'Select an npm script';
        quickPick.items = taskList;
        const toDispose = [];
        let pickPromise = new Promise((c) => {
            toDispose.push(quickPick.onDidAccept(() => {
                toDispose.forEach(d => d.dispose());
                c(quickPick.selectedItems[0]);
            }));
            toDispose.push(quickPick.onDidHide(() => {
                toDispose.forEach(d => d.dispose());
                c(undefined);
            }));
        });
        quickPick.show();
        let result = await pickPromise;
        quickPick.dispose();
        if (result) {
            vscode.tasks.executeTask(result.task);
        }
    }
    else {
        vscode.window.showInformationMessage(`No npm scripts found in ${selectedFolder.fsPath}`, { modal: true });
    }
}
exports.selectAndRunScriptFromFolder = selectAndRunScriptFromFolder;
//# sourceMappingURL=commands.js.map