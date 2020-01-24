"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const vscode_1 = require("vscode");
const util_1 = require("../util");
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
    get onDidOpenRepository() {
        return util_1.mapEvent(this._model.onDidOpenRepository, r => new ApiRepository(r));
    }
    get onDidCloseRepository() {
        return util_1.mapEvent(this._model.onDidCloseRepository, r => new ApiRepository(r));
    }
    get repositories() {
        return this._model.repositories.map(r => new ApiRepository(r));
    }
}
exports.ApiImpl = ApiImpl;
//# sourceMappingURL=api1.js.map