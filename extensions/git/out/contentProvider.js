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
const vscode_1 = require("vscode");
const decorators_1 = require("./decorators");
const uri_1 = require("./uri");
const util_1 = require("./util");
const THREE_MINUTES = 1000 * 60 * 3;
const FIVE_MINUTES = 1000 * 60 * 5;
class GitContentProvider {
    constructor(model) {
        this.model = model;
        this._onDidChange = new vscode_1.EventEmitter();
        this.changedRepositoryRoots = new Set();
        this.cache = Object.create(null);
        this.disposables = [];
        this.disposables.push(model.onDidChangeRepository(this.onDidChangeRepository, this), model.onDidChangeOriginalResource(this.onDidChangeOriginalResource, this), vscode_1.workspace.registerTextDocumentContentProvider('git', this));
        setInterval(() => this.cleanup(), FIVE_MINUTES);
    }
    get onDidChange() { return this._onDidChange.event; }
    onDidChangeRepository({ repository }) {
        this.changedRepositoryRoots.add(repository.root);
        this.eventuallyFireChangeEvents();
    }
    onDidChangeOriginalResource({ uri }) {
        if (uri.scheme !== 'file') {
            return;
        }
        this._onDidChange.fire(uri_1.toGitUri(uri, '', { replaceFileExtension: true }));
    }
    eventuallyFireChangeEvents() {
        this.fireChangeEvents();
    }
    async fireChangeEvents() {
        if (!vscode_1.window.state.focused) {
            const onDidFocusWindow = util_1.filterEvent(vscode_1.window.onDidChangeWindowState, e => e.focused);
            await util_1.eventToPromise(onDidFocusWindow);
        }
        Object.keys(this.cache).forEach(key => {
            const uri = this.cache[key].uri;
            const fsPath = uri.fsPath;
            for (const root of this.changedRepositoryRoots) {
                if (util_1.isDescendant(root, fsPath)) {
                    this._onDidChange.fire(uri);
                    return;
                }
            }
        });
        this.changedRepositoryRoots.clear();
    }
    async provideTextDocumentContent(uri) {
        let { path, ref, submoduleOf } = uri_1.fromGitUri(uri);
        if (submoduleOf) {
            const repository = this.model.getRepository(submoduleOf);
            if (!repository) {
                return '';
            }
            if (ref === 'index') {
                return await repository.diffIndexWithHEAD(path);
            }
            else {
                return await repository.diffWithHEAD(path);
            }
        }
        const repository = this.model.getRepository(uri);
        if (!repository) {
            return '';
        }
        const cacheKey = uri.toString();
        const timestamp = new Date().getTime();
        const cacheValue = { uri, timestamp };
        this.cache[cacheKey] = cacheValue;
        if (ref === '~') {
            const fileUri = vscode_1.Uri.file(path);
            const uriString = fileUri.toString();
            const [indexStatus] = repository.indexGroup.resourceStates.filter(r => r.resourceUri.toString() === uriString);
            ref = indexStatus ? '' : 'HEAD';
        }
        else if (/^~\d$/.test(ref)) {
            ref = `:${ref[1]}`;
        }
        try {
            return await repository.show(ref, path);
        }
        catch (err) {
            return '';
        }
    }
    cleanup() {
        const now = new Date().getTime();
        const cache = Object.create(null);
        Object.keys(this.cache).forEach(key => {
            const row = this.cache[key];
            const { path } = uri_1.fromGitUri(row.uri);
            const isOpen = vscode_1.workspace.textDocuments
                .filter(d => d.uri.scheme === 'file')
                .some(d => util_1.pathEquals(d.uri.fsPath, path));
            if (isOpen || now - row.timestamp < THREE_MINUTES) {
                cache[row.uri.toString()] = row;
            }
        });
        this.cache = cache;
    }
    dispose() {
        this.disposables.forEach(d => d.dispose());
    }
}
__decorate([
    decorators_1.debounce(1100)
], GitContentProvider.prototype, "eventuallyFireChangeEvents", null);
__decorate([
    decorators_1.throttle
], GitContentProvider.prototype, "fireChangeEvents", null);
exports.GitContentProvider = GitContentProvider;
//# sourceMappingURL=contentProvider.js.map