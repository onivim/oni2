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
exports.GitDecorations = void 0;
const vscode_1 = require("vscode");
const path = require("path");
const decorators_1 = require("./decorators");
const util_1 = require("./util");
class GitIgnoreDecorationProvider {
    constructor(model) {
        this.model = model;
        this.queue = new Map();
        this.disposables = [];
        this.onDidChangeFileDecorations = util_1.fireEvent(util_1.anyEvent(util_1.filterEvent(vscode_1.workspace.onDidSaveTextDocument, e => /\.gitignore$|\.git\/info\/exclude$/.test(e.uri.path)), model.onDidOpenRepository, model.onDidCloseRepository));
        this.disposables.push(vscode_1.window.registerFileDecorationProvider(this));
    }
    async provideFileDecoration(uri) {
        const repository = this.model.getRepository(uri);
        if (!repository) {
            return;
        }
        let queueItem = this.queue.get(repository.root);
        if (!queueItem) {
            queueItem = { repository, queue: new Map() };
            this.queue.set(repository.root, queueItem);
        }
        let promiseSource = queueItem.queue.get(uri.fsPath);
        if (!promiseSource) {
            promiseSource = new util_1.PromiseSource();
            queueItem.queue.set(uri.fsPath, promiseSource);
            this.checkIgnoreSoon();
        }
        return await promiseSource.promise;
    }
    checkIgnoreSoon() {
        const queue = new Map(this.queue.entries());
        this.queue.clear();
        for (const [, item] of queue) {
            const paths = [...item.queue.keys()];
            item.repository.checkIgnore(paths).then(ignoreSet => {
                for (const [path, promiseSource] of item.queue.entries()) {
                    promiseSource.resolve(ignoreSet.has(path) ? GitIgnoreDecorationProvider.Decoration : undefined);
                }
            }, err => {
                if (err.gitErrorCode !== "IsInSubmodule" /* IsInSubmodule */) {
                    console.error(err);
                }
                for (const [, promiseSource] of item.queue.entries()) {
                    promiseSource.reject(err);
                }
            });
        }
    }
    dispose() {
        this.disposables.forEach(d => d.dispose());
        this.queue.clear();
    }
}
GitIgnoreDecorationProvider.Decoration = { color: new vscode_1.ThemeColor('gitDecoration.ignoredResourceForeground') };
__decorate([
    decorators_1.debounce(500)
], GitIgnoreDecorationProvider.prototype, "checkIgnoreSoon", null);
class GitDecorationProvider {
    constructor(repository) {
        this.repository = repository;
        this._onDidChangeDecorations = new vscode_1.EventEmitter();
        this.onDidChangeFileDecorations = this._onDidChangeDecorations.event;
        this.disposables = [];
        this.decorations = new Map();
        this.disposables.push(vscode_1.window.registerFileDecorationProvider(this), repository.onDidRunGitStatus(this.onDidRunGitStatus, this));
    }
    onDidRunGitStatus() {
        let newDecorations = new Map();
        this.collectSubmoduleDecorationData(newDecorations);
        this.collectDecorationData(this.repository.indexGroup, newDecorations);
        this.collectDecorationData(this.repository.untrackedGroup, newDecorations);
        this.collectDecorationData(this.repository.workingTreeGroup, newDecorations);
        this.collectDecorationData(this.repository.mergeGroup, newDecorations);
        const uris = new Set([...this.decorations.keys()].concat([...newDecorations.keys()]));
        this.decorations = newDecorations;
        this._onDidChangeDecorations.fire([...uris.values()].map(value => vscode_1.Uri.parse(value, true)));
    }
    collectDecorationData(group, bucket) {
        for (const r of group.resourceStates) {
            const decoration = r.resourceDecoration;
            if (decoration) {
                // not deleted and has a decoration
                bucket.set(r.original.toString(), decoration);
                if (r.type === 3 /* INDEX_RENAMED */) {
                    bucket.set(r.resourceUri.toString(), decoration);
                }
            }
        }
    }
    collectSubmoduleDecorationData(bucket) {
        for (const submodule of this.repository.submodules) {
            bucket.set(vscode_1.Uri.file(path.join(this.repository.root, submodule.path)).toString(), GitDecorationProvider.SubmoduleDecorationData);
        }
    }
    provideFileDecoration(uri) {
        return this.decorations.get(uri.toString());
    }
    dispose() {
        this.disposables.forEach(d => d.dispose());
    }
}
GitDecorationProvider.SubmoduleDecorationData = {
    tooltip: 'Submodule',
    badge: 'S',
    color: new vscode_1.ThemeColor('gitDecoration.submoduleResourceForeground')
};
class GitDecorations {
    constructor(model) {
        this.model = model;
        this.disposables = [];
        this.modelDisposables = [];
        this.providers = new Map();
        this.disposables.push(new GitIgnoreDecorationProvider(model));
        const onEnablementChange = util_1.filterEvent(vscode_1.workspace.onDidChangeConfiguration, e => e.affectsConfiguration('git.decorations.enabled'));
        onEnablementChange(this.update, this, this.disposables);
        this.update();
    }
    update() {
        const enabled = vscode_1.workspace.getConfiguration('git').get('decorations.enabled');
        if (enabled) {
            this.enable();
        }
        else {
            this.disable();
        }
    }
    enable() {
        this.model.onDidOpenRepository(this.onDidOpenRepository, this, this.modelDisposables);
        this.model.onDidCloseRepository(this.onDidCloseRepository, this, this.modelDisposables);
        this.model.repositories.forEach(this.onDidOpenRepository, this);
    }
    disable() {
        this.modelDisposables = util_1.dispose(this.modelDisposables);
        this.providers.forEach(value => value.dispose());
        this.providers.clear();
    }
    onDidOpenRepository(repository) {
        const provider = new GitDecorationProvider(repository);
        this.providers.set(repository, provider);
    }
    onDidCloseRepository(repository) {
        const provider = this.providers.get(repository);
        if (provider) {
            provider.dispose();
            this.providers.delete(repository);
        }
    }
    dispose() {
        this.disable();
        this.disposables = util_1.dispose(this.disposables);
    }
}
exports.GitDecorations = GitDecorations;
//# sourceMappingURL=decorationProvider.js.map