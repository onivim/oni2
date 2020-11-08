"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
require("mocha");
const assert = require("assert");
const vscode_1 = require("vscode");
const cp = require("child_process");
const fs = require("fs");
const path = require("path");
const util_1 = require("../util");
suite('git smoke test', function () {
    const cwd = fs.realpathSync(vscode_1.workspace.workspaceFolders[0].uri.fsPath);
    function file(relativePath) {
        return path.join(cwd, relativePath);
    }
    function uri(relativePath) {
        return vscode_1.Uri.file(file(relativePath));
    }
    async function open(relativePath) {
        const doc = await vscode_1.workspace.openTextDocument(uri(relativePath));
        await vscode_1.window.showTextDocument(doc);
        return doc;
    }
    async function type(doc, text) {
        const edit = new vscode_1.WorkspaceEdit();
        const end = doc.lineAt(doc.lineCount - 1).range.end;
        edit.replace(doc.uri, new vscode_1.Range(end, end), text);
        await vscode_1.workspace.applyEdit(edit);
    }
    let git;
    let repository;
    suiteSetup(async function () {
        fs.writeFileSync(file('app.js'), 'hello', 'utf8');
        fs.writeFileSync(file('index.pug'), 'hello', 'utf8');
        cp.execSync('git init', { cwd });
        cp.execSync('git config user.name testuser', { cwd });
        cp.execSync('git config user.email monacotools@microsoft.com', { cwd });
        cp.execSync('git add .', { cwd });
        cp.execSync('git commit -m "initial commit"', { cwd });
        // make sure git is activated
        const ext = vscode_1.extensions.getExtension('vscode.git');
        await (ext === null || ext === void 0 ? void 0 : ext.activate());
        git = ext.exports.getAPI(1);
        if (git.repositories.length === 0) {
            await util_1.eventToPromise(git.onDidOpenRepository);
        }
        assert.equal(git.repositories.length, 1);
        assert.equal(fs.realpathSync(git.repositories[0].rootUri.fsPath), cwd);
        repository = git.repositories[0];
    });
    test('reflects working tree changes', async function () {
        await vscode_1.commands.executeCommand('workbench.view.scm');
        const appjs = await open('app.js');
        await type(appjs, ' world');
        await appjs.save();
        await repository.status();
        assert.equal(repository.state.workingTreeChanges.length, 1);
        repository.state.workingTreeChanges.some(r => r.uri.path === appjs.uri.path && r.status === 5 /* MODIFIED */);
        fs.writeFileSync(file('newfile.txt'), '');
        const newfile = await open('newfile.txt');
        await type(newfile, 'hey there');
        await newfile.save();
        await repository.status();
        assert.equal(repository.state.workingTreeChanges.length, 2);
        repository.state.workingTreeChanges.some(r => r.uri.path === appjs.uri.path && r.status === 5 /* MODIFIED */);
        repository.state.workingTreeChanges.some(r => r.uri.path === newfile.uri.path && r.status === 7 /* UNTRACKED */);
    });
    test('opens diff editor', async function () {
        const appjs = uri('app.js');
        await vscode_1.commands.executeCommand('git.openChange', appjs);
        assert(vscode_1.window.activeTextEditor);
        assert.equal(vscode_1.window.activeTextEditor.document.uri.path, appjs.path);
        // TODO: how do we really know this is a diff editor?
    });
    test('stages correctly', async function () {
        const appjs = uri('app.js');
        const newfile = uri('newfile.txt');
        await vscode_1.commands.executeCommand('git.stage', appjs);
        assert.equal(repository.state.workingTreeChanges.length, 1);
        repository.state.workingTreeChanges.some(r => r.uri.path === newfile.path && r.status === 7 /* UNTRACKED */);
        assert.equal(repository.state.indexChanges.length, 1);
        repository.state.indexChanges.some(r => r.uri.path === appjs.path && r.status === 0 /* INDEX_MODIFIED */);
        await vscode_1.commands.executeCommand('git.unstage', appjs);
        assert.equal(repository.state.workingTreeChanges.length, 2);
        repository.state.workingTreeChanges.some(r => r.uri.path === appjs.path && r.status === 5 /* MODIFIED */);
        repository.state.workingTreeChanges.some(r => r.uri.path === newfile.path && r.status === 7 /* UNTRACKED */);
    });
    test('stages, commits changes and verifies outgoing change', async function () {
        const appjs = uri('app.js');
        const newfile = uri('newfile.txt');
        await vscode_1.commands.executeCommand('git.stage', appjs);
        await repository.commit('second commit');
        assert.equal(repository.state.workingTreeChanges.length, 1);
        repository.state.workingTreeChanges.some(r => r.uri.path === newfile.path && r.status === 7 /* UNTRACKED */);
        assert.equal(repository.state.indexChanges.length, 0);
        await vscode_1.commands.executeCommand('git.stageAll', appjs);
        await repository.commit('third commit');
        assert.equal(repository.state.workingTreeChanges.length, 0);
        assert.equal(repository.state.indexChanges.length, 0);
    });
});
//# sourceMappingURL=smoke.test.js.map