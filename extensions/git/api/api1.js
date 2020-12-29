"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.registerAPICommands = exports.ApiImpl = exports.ApiGit = exports.ApiRepository = exports.ApiRepositoryUIState = exports.ApiRepositoryState = exports.ApiChange = void 0;
const vscode_1 = require("vscode");
const util_1 = require("../util");
const uri_1 = require("../uri");
const remoteSource_1 = require("../remoteSource");
class ApiInputBox {
    constructor(_inputBox) {
        this._inputBox = _inputBox;
    }
    set value(value) { this._inputBox.value = value; }
    get value() { return this._inputBox.value; }
}
class ApiChange {
    constructor(resource) {
        this.resource = resource;
    }
    get uri() { return this.resource.resourceUri; }
    get originalUri() { return this.resource.original; }
    get renameUri() { return this.resource.renameResourceUri; }
    get status() { return this.resource.type; }
}
exports.ApiChange = ApiChange;
class ApiRepositoryState {
    constructor(_repository) {
        this._repository = _repository;
        this.onDidChange = this._repository.onDidRunGitStatus;
    }
    get HEAD() { return this._repository.HEAD; }
    get refs() { return [...this._repository.refs]; }
    get remotes() { return [...this._repository.remotes]; }
    get submodules() { return [...this._repository.submodules]; }
    get rebaseCommit() { return this._repository.rebaseCommit; }
    get mergeChanges() { return this._repository.mergeGroup.resourceStates.map(r => new ApiChange(r)); }
    get indexChanges() { return this._repository.indexGroup.resourceStates.map(r => new ApiChange(r)); }
    get workingTreeChanges() { return this._repository.workingTreeGroup.resourceStates.map(r => new ApiChange(r)); }
}
exports.ApiRepositoryState = ApiRepositoryState;
class ApiRepositoryUIState {
    constructor(_sourceControl) {
        this._sourceControl = _sourceControl;
        this.onDidChange = util_1.mapEvent(this._sourceControl.onDidChangeSelection, () => null);
    }
    get selected() { return this._sourceControl.selected; }
}
exports.ApiRepositoryUIState = ApiRepositoryUIState;
class ApiRepository {
    constructor(_repository) {
        this._repository = _repository;
        this.rootUri = vscode_1.Uri.file(this._repository.root);
        this.inputBox = new ApiInputBox(this._repository.inputBox);
        this.state = new ApiRepositoryState(this._repository);
        this.ui = new ApiRepositoryUIState(this._repository.sourceControl);
    }
    apply(patch, reverse) {
        return this._repository.apply(patch, reverse);
    }
    getConfigs() {
        return this._repository.getConfigs();
    }
    getConfig(key) {
        return this._repository.getConfig(key);
    }
    setConfig(key, value) {
        return this._repository.setConfig(key, value);
    }
    getGlobalConfig(key) {
        return this._repository.getGlobalConfig(key);
    }
    getObjectDetails(treeish, path) {
        return this._repository.getObjectDetails(treeish, path);
    }
    detectObjectType(object) {
        return this._repository.detectObjectType(object);
    }
    buffer(ref, filePath) {
        return this._repository.buffer(ref, filePath);
    }
    show(ref, path) {
        return this._repository.show(ref, path);
    }
    getCommit(ref) {
        return this._repository.getCommit(ref);
    }
    clean(paths) {
        return this._repository.clean(paths.map(p => vscode_1.Uri.file(p)));
    }
    diff(cached) {
        return this._repository.diff(cached);
    }
    diffWithHEAD(path) {
        return this._repository.diffWithHEAD(path);
    }
    diffWith(ref, path) {
        return this._repository.diffWith(ref, path);
    }
    diffIndexWithHEAD(path) {
        return this._repository.diffIndexWithHEAD(path);
    }
    diffIndexWith(ref, path) {
        return this._repository.diffIndexWith(ref, path);
    }
    diffBlobs(object1, object2) {
        return this._repository.diffBlobs(object1, object2);
    }
    diffBetween(ref1, ref2, path) {
        return this._repository.diffBetween(ref1, ref2, path);
    }
    hashObject(data) {
        return this._repository.hashObject(data);
    }
    createBranch(name, checkout, ref) {
        return this._repository.branch(name, checkout, ref);
    }
    deleteBranch(name, force) {
        return this._repository.deleteBranch(name, force);
    }
    getBranch(name) {
        return this._repository.getBranch(name);
    }
    getBranches(query) {
        return this._repository.getBranches(query);
    }
    setBranchUpstream(name, upstream) {
        return this._repository.setBranchUpstream(name, upstream);
    }
    getMergeBase(ref1, ref2) {
        return this._repository.getMergeBase(ref1, ref2);
    }
    status() {
        return this._repository.status();
    }
    checkout(treeish) {
        return this._repository.checkout(treeish);
    }
    addRemote(name, url) {
        return this._repository.addRemote(name, url);
    }
    removeRemote(name) {
        return this._repository.removeRemote(name);
    }
    renameRemote(name, newName) {
        return this._repository.renameRemote(name, newName);
    }
    fetch(remote, ref, depth) {
        return this._repository.fetch(remote, ref, depth);
    }
    pull(unshallow) {
        return this._repository.pull(undefined, unshallow);
    }
    push(remoteName, branchName, setUpstream = false) {
        return this._repository.pushTo(remoteName, branchName, setUpstream);
    }
    blame(path) {
        return this._repository.blame(path);
    }
    log(options) {
        return this._repository.log(options);
    }
    commit(message, opts) {
        return this._repository.commit(message, opts);
    }
}
exports.ApiRepository = ApiRepository;
class ApiGit {
    constructor(_model) {
        this._model = _model;
    }
    get path() { return this._model.git.path; }
}
exports.ApiGit = ApiGit;
class ApiImpl {
    constructor(_model) {
        this._model = _model;
        this.git = new ApiGit(this._model);
    }
    get state() {
        return this._model.state;
    }
    get onDidChangeState() {
        return this._model.onDidChangeState;
    }
    get onDidOpenRepository() {
        return util_1.mapEvent(this._model.onDidOpenRepository, r => new ApiRepository(r));
    }
    get onDidCloseRepository() {
        return util_1.mapEvent(this._model.onDidCloseRepository, r => new ApiRepository(r));
    }
    get repositories() {
        return this._model.repositories.map(r => new ApiRepository(r));
    }
    toGitUri(uri, ref) {
        return uri_1.toGitUri(uri, ref);
    }
    getRepository(uri) {
        const result = this._model.getRepository(uri);
        return result ? new ApiRepository(result) : null;
    }
    async init(root) {
        const path = root.fsPath;
        await this._model.git.init(path);
        await this._model.openRepository(path);
        return this.getRepository(root) || null;
    }
    registerRemoteSourceProvider(provider) {
        return this._model.registerRemoteSourceProvider(provider);
    }
    registerCredentialsProvider(provider) {
        return this._model.registerCredentialsProvider(provider);
    }
    registerPushErrorHandler(handler) {
        return this._model.registerPushErrorHandler(handler);
    }
}
exports.ApiImpl = ApiImpl;
function getRefType(type) {
    switch (type) {
        case 0 /* Head */: return 'Head';
        case 1 /* RemoteHead */: return 'RemoteHead';
        case 2 /* Tag */: return 'Tag';
    }
    return 'unknown';
}
function getStatus(status) {
    switch (status) {
        case 0 /* INDEX_MODIFIED */: return 'INDEX_MODIFIED';
        case 1 /* INDEX_ADDED */: return 'INDEX_ADDED';
        case 2 /* INDEX_DELETED */: return 'INDEX_DELETED';
        case 3 /* INDEX_RENAMED */: return 'INDEX_RENAMED';
        case 4 /* INDEX_COPIED */: return 'INDEX_COPIED';
        case 5 /* MODIFIED */: return 'MODIFIED';
        case 6 /* DELETED */: return 'DELETED';
        case 7 /* UNTRACKED */: return 'UNTRACKED';
        case 8 /* IGNORED */: return 'IGNORED';
        case 9 /* INTENT_TO_ADD */: return 'INTENT_TO_ADD';
        case 10 /* ADDED_BY_US */: return 'ADDED_BY_US';
        case 11 /* ADDED_BY_THEM */: return 'ADDED_BY_THEM';
        case 12 /* DELETED_BY_US */: return 'DELETED_BY_US';
        case 13 /* DELETED_BY_THEM */: return 'DELETED_BY_THEM';
        case 14 /* BOTH_ADDED */: return 'BOTH_ADDED';
        case 15 /* BOTH_DELETED */: return 'BOTH_DELETED';
        case 16 /* BOTH_MODIFIED */: return 'BOTH_MODIFIED';
    }
    return 'UNKNOWN';
}
function registerAPICommands(extension) {
    const disposables = [];
    disposables.push(vscode_1.commands.registerCommand('git.api.getRepositories', () => {
        const api = extension.getAPI(1);
        return api.repositories.map(r => r.rootUri.toString());
    }));
    disposables.push(vscode_1.commands.registerCommand('git.api.getRepositoryState', (uri) => {
        const api = extension.getAPI(1);
        const repository = api.getRepository(vscode_1.Uri.parse(uri));
        if (!repository) {
            return null;
        }
        const state = repository.state;
        const ref = (ref) => (ref && { ...ref, type: getRefType(ref.type) });
        const change = (change) => {
            var _a;
            return ({
                uri: change.uri.toString(),
                originalUri: change.originalUri.toString(),
                renameUri: (_a = change.renameUri) === null || _a === void 0 ? void 0 : _a.toString(),
                status: getStatus(change.status)
            });
        };
        return {
            HEAD: ref(state.HEAD),
            refs: state.refs.map(ref),
            remotes: state.remotes,
            submodules: state.submodules,
            rebaseCommit: state.rebaseCommit,
            mergeChanges: state.mergeChanges.map(change),
            indexChanges: state.indexChanges.map(change),
            workingTreeChanges: state.workingTreeChanges.map(change)
        };
    }));
    disposables.push(vscode_1.commands.registerCommand('git.api.getRemoteSources', (opts) => {
        if (!extension.model) {
            return;
        }
        return remoteSource_1.pickRemoteSource(extension.model, opts);
    }));
    return vscode_1.Disposable.from(...disposables);
}
exports.registerAPICommands = registerAPICommands;
//# sourceMappingURL=api1.js.map