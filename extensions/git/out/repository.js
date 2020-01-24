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
const git_1 = require("./git");
const util_1 = require("./util");
const decorators_1 = require("./decorators");
const uri_1 = require("./uri");
const autofetch_1 = require("./autofetch");
const path = require("path");
const nls = require("vscode-nls");
const fs = require("fs");
const statusbar_1 = require("./statusbar");
const timeout = (millis) => new Promise(c => setTimeout(c, millis));
const localize = nls.loadMessageBundle();
const iconsRootPath = path.join(path.dirname(__dirname), 'resources', 'icons');
function getIconUri(iconName, theme) {
    return vscode_1.Uri.file(path.join(iconsRootPath, theme, `${iconName}.svg`));
}
class Resource {
    constructor(_resourceGroupType, _resourceUri, _type, _useIcons, _renameResourceUri) {
        this._resourceGroupType = _resourceGroupType;
        this._resourceUri = _resourceUri;
        this._type = _type;
        this._useIcons = _useIcons;
        this._renameResourceUri = _renameResourceUri;
    }
    get resourceUri() {
        if (this.renameResourceUri && (this._type === 5 /* MODIFIED */ || this._type === 6 /* DELETED */ || this._type === 3 /* INDEX_RENAMED */ || this._type === 4 /* INDEX_COPIED */)) {
            return this.renameResourceUri;
        }
        return this._resourceUri;
    }
    get command() {
        return {
            command: 'git.openResource',
            title: localize('open', "Open"),
            arguments: [this]
        };
    }
    get resourceGroupType() { return this._resourceGroupType; }
    get type() { return this._type; }
    get original() { return this._resourceUri; }
    get renameResourceUri() { return this._renameResourceUri; }
    getIconPath(theme) {
        switch (this.type) {
            case 0 /* INDEX_MODIFIED */: return Resource.Icons[theme].Modified;
            case 5 /* MODIFIED */: return Resource.Icons[theme].Modified;
            case 1 /* INDEX_ADDED */: return Resource.Icons[theme].Added;
            case 2 /* INDEX_DELETED */: return Resource.Icons[theme].Deleted;
            case 6 /* DELETED */: return Resource.Icons[theme].Deleted;
            case 3 /* INDEX_RENAMED */: return Resource.Icons[theme].Renamed;
            case 4 /* INDEX_COPIED */: return Resource.Icons[theme].Copied;
            case 7 /* UNTRACKED */: return Resource.Icons[theme].Untracked;
            case 8 /* IGNORED */: return Resource.Icons[theme].Ignored;
            case 9 /* INTENT_TO_ADD */: return Resource.Icons[theme].Added;
            case 15 /* BOTH_DELETED */: return Resource.Icons[theme].Conflict;
            case 10 /* ADDED_BY_US */: return Resource.Icons[theme].Conflict;
            case 13 /* DELETED_BY_THEM */: return Resource.Icons[theme].Conflict;
            case 11 /* ADDED_BY_THEM */: return Resource.Icons[theme].Conflict;
            case 12 /* DELETED_BY_US */: return Resource.Icons[theme].Conflict;
            case 14 /* BOTH_ADDED */: return Resource.Icons[theme].Conflict;
            case 16 /* BOTH_MODIFIED */: return Resource.Icons[theme].Conflict;
            default: throw new Error('Unknown git status: ' + this.type);
        }
    }
    get tooltip() {
        switch (this.type) {
            case 0 /* INDEX_MODIFIED */: return localize('index modified', "Index Modified");
            case 5 /* MODIFIED */: return localize('modified', "Modified");
            case 1 /* INDEX_ADDED */: return localize('index added', "Index Added");
            case 2 /* INDEX_DELETED */: return localize('index deleted', "Index Deleted");
            case 6 /* DELETED */: return localize('deleted', "Deleted");
            case 3 /* INDEX_RENAMED */: return localize('index renamed', "Index Renamed");
            case 4 /* INDEX_COPIED */: return localize('index copied', "Index Copied");
            case 7 /* UNTRACKED */: return localize('untracked', "Untracked");
            case 8 /* IGNORED */: return localize('ignored', "Ignored");
            case 9 /* INTENT_TO_ADD */: return localize('intent to add', "Intent to Add");
            case 15 /* BOTH_DELETED */: return localize('both deleted', "Both Deleted");
            case 10 /* ADDED_BY_US */: return localize('added by us', "Added By Us");
            case 13 /* DELETED_BY_THEM */: return localize('deleted by them', "Deleted By Them");
            case 11 /* ADDED_BY_THEM */: return localize('added by them', "Added By Them");
            case 12 /* DELETED_BY_US */: return localize('deleted by us', "Deleted By Us");
            case 14 /* BOTH_ADDED */: return localize('both added', "Both Added");
            case 16 /* BOTH_MODIFIED */: return localize('both modified', "Both Modified");
            default: return '';
        }
    }
    get strikeThrough() {
        switch (this.type) {
            case 6 /* DELETED */:
            case 15 /* BOTH_DELETED */:
            case 13 /* DELETED_BY_THEM */:
            case 12 /* DELETED_BY_US */:
            case 2 /* INDEX_DELETED */:
                return true;
            default:
                return false;
        }
    }
    get faded() {
        // TODO@joao
        return false;
        // const workspaceRootPath = this.workspaceRoot.fsPath;
        // return this.resourceUri.fsPath.substr(0, workspaceRootPath.length) !== workspaceRootPath;
    }
    get decorations() {
        const light = this._useIcons ? { iconPath: this.getIconPath('light') } : undefined;
        const dark = this._useIcons ? { iconPath: this.getIconPath('dark') } : undefined;
        const tooltip = this.tooltip;
        const strikeThrough = this.strikeThrough;
        const faded = this.faded;
        const letter = this.letter;
        const color = this.color;
        return { strikeThrough, faded, tooltip, light, dark, letter, color, source: 'git.resource' /*todo@joh*/ };
    }
    get letter() {
        switch (this.type) {
            case 0 /* INDEX_MODIFIED */:
            case 5 /* MODIFIED */:
                return 'M';
            case 1 /* INDEX_ADDED */:
            case 9 /* INTENT_TO_ADD */:
                return 'A';
            case 2 /* INDEX_DELETED */:
            case 6 /* DELETED */:
                return 'D';
            case 3 /* INDEX_RENAMED */:
                return 'R';
            case 7 /* UNTRACKED */:
                return 'U';
            case 8 /* IGNORED */:
                return 'I';
            case 13 /* DELETED_BY_THEM */:
                return 'D';
            case 12 /* DELETED_BY_US */:
                return 'D';
            case 4 /* INDEX_COPIED */:
            case 15 /* BOTH_DELETED */:
            case 10 /* ADDED_BY_US */:
            case 11 /* ADDED_BY_THEM */:
            case 14 /* BOTH_ADDED */:
            case 16 /* BOTH_MODIFIED */:
                return 'C';
            default:
                throw new Error('Unknown git status: ' + this.type);
        }
    }
    get color() {
        switch (this.type) {
            case 0 /* INDEX_MODIFIED */:
            case 5 /* MODIFIED */:
                return new vscode_1.ThemeColor('gitDecoration.modifiedResourceForeground');
            case 2 /* INDEX_DELETED */:
            case 6 /* DELETED */:
                return new vscode_1.ThemeColor('gitDecoration.deletedResourceForeground');
            case 1 /* INDEX_ADDED */:
            case 9 /* INTENT_TO_ADD */:
                return new vscode_1.ThemeColor('gitDecoration.addedResourceForeground');
            case 3 /* INDEX_RENAMED */:
            case 7 /* UNTRACKED */:
                return new vscode_1.ThemeColor('gitDecoration.untrackedResourceForeground');
            case 8 /* IGNORED */:
                return new vscode_1.ThemeColor('gitDecoration.ignoredResourceForeground');
            case 4 /* INDEX_COPIED */:
            case 15 /* BOTH_DELETED */:
            case 10 /* ADDED_BY_US */:
            case 13 /* DELETED_BY_THEM */:
            case 11 /* ADDED_BY_THEM */:
            case 12 /* DELETED_BY_US */:
            case 14 /* BOTH_ADDED */:
            case 16 /* BOTH_MODIFIED */:
                return new vscode_1.ThemeColor('gitDecoration.conflictingResourceForeground');
            default:
                throw new Error('Unknown git status: ' + this.type);
        }
    }
    get priority() {
        switch (this.type) {
            case 0 /* INDEX_MODIFIED */:
            case 5 /* MODIFIED */:
                return 2;
            case 8 /* IGNORED */:
                return 3;
            case 4 /* INDEX_COPIED */:
            case 15 /* BOTH_DELETED */:
            case 10 /* ADDED_BY_US */:
            case 13 /* DELETED_BY_THEM */:
            case 11 /* ADDED_BY_THEM */:
            case 12 /* DELETED_BY_US */:
            case 14 /* BOTH_ADDED */:
            case 16 /* BOTH_MODIFIED */:
                return 4;
            default:
                return 1;
        }
    }
    get resourceDecoration() {
        const title = this.tooltip;
        const letter = this.letter;
        const color = this.color;
        const priority = this.priority;
        return { bubble: true, source: 'git.resource', title, letter, color, priority };
    }
}
Resource.Icons = {
    light: {
        Modified: getIconUri('status-modified', 'light'),
        Added: getIconUri('status-added', 'light'),
        Deleted: getIconUri('status-deleted', 'light'),
        Renamed: getIconUri('status-renamed', 'light'),
        Copied: getIconUri('status-copied', 'light'),
        Untracked: getIconUri('status-untracked', 'light'),
        Ignored: getIconUri('status-ignored', 'light'),
        Conflict: getIconUri('status-conflict', 'light'),
    },
    dark: {
        Modified: getIconUri('status-modified', 'dark'),
        Added: getIconUri('status-added', 'dark'),
        Deleted: getIconUri('status-deleted', 'dark'),
        Renamed: getIconUri('status-renamed', 'dark'),
        Copied: getIconUri('status-copied', 'dark'),
        Untracked: getIconUri('status-untracked', 'dark'),
        Ignored: getIconUri('status-ignored', 'dark'),
        Conflict: getIconUri('status-conflict', 'dark')
    }
};
__decorate([
    decorators_1.memoize
], Resource.prototype, "resourceUri", null);
__decorate([
    decorators_1.memoize
], Resource.prototype, "command", null);
__decorate([
    decorators_1.memoize
], Resource.prototype, "faded", null);
exports.Resource = Resource;
function isReadOnly(operation) {
    switch (operation) {
        case "Show" /* Show */:
        case "GetCommitTemplate" /* GetCommitTemplate */:
        case "CheckIgnore" /* CheckIgnore */:
        case "GetObjectDetails" /* GetObjectDetails */:
        case "MergeBase" /* MergeBase */:
            return true;
        default:
            return false;
    }
}
function shouldShowProgress(operation) {
    switch (operation) {
        case "Fetch" /* Fetch */:
        case "CheckIgnore" /* CheckIgnore */:
        case "GetObjectDetails" /* GetObjectDetails */:
        case "Show" /* Show */:
            return false;
        default:
            return true;
    }
}
class OperationsImpl {
    constructor() {
        this.operations = new Map();
    }
    start(operation) {
        this.operations.set(operation, (this.operations.get(operation) || 0) + 1);
    }
    end(operation) {
        const count = (this.operations.get(operation) || 0) - 1;
        if (count <= 0) {
            this.operations.delete(operation);
        }
        else {
            this.operations.set(operation, count);
        }
    }
    isRunning(operation) {
        return this.operations.has(operation);
    }
    isIdle() {
        const operations = this.operations.keys();
        for (const operation of operations) {
            if (!isReadOnly(operation)) {
                return false;
            }
        }
        return true;
    }
    shouldShowProgress() {
        const operations = this.operations.keys();
        for (const operation of operations) {
            if (shouldShowProgress(operation)) {
                return true;
            }
        }
        return false;
    }
}
class ProgressManager {
    constructor(repository) {
        this.repository = repository;
        this.enabled = false;
        this.disposable = util_1.EmptyDisposable;
        const onDidChange = util_1.filterEvent(vscode_1.workspace.onDidChangeConfiguration, e => e.affectsConfiguration('git', vscode_1.Uri.file(this.repository.root)));
        onDidChange(_ => this.updateEnablement());
        this.updateEnablement();
    }
    updateEnablement() {
        const config = vscode_1.workspace.getConfiguration('git', vscode_1.Uri.file(this.repository.root));
        if (config.get('showProgress')) {
            this.enable();
        }
        else {
            this.disable();
        }
    }
    enable() {
        if (this.enabled) {
            return;
        }
        const start = util_1.onceEvent(util_1.filterEvent(this.repository.onDidChangeOperations, () => this.repository.operations.shouldShowProgress()));
        const end = util_1.onceEvent(util_1.filterEvent(util_1.debounceEvent(this.repository.onDidChangeOperations, 300), () => !this.repository.operations.shouldShowProgress()));
        const setup = () => {
            this.disposable = start(() => {
                const promise = util_1.eventToPromise(end).then(() => setup());
                vscode_1.window.withProgress({ location: vscode_1.ProgressLocation.SourceControl }, () => promise);
            });
        };
        setup();
        this.enabled = true;
    }
    disable() {
        if (!this.enabled) {
            return;
        }
        this.disposable.dispose();
        this.disposable = util_1.EmptyDisposable;
        this.enabled = false;
    }
    dispose() {
        this.disable();
    }
}
class Repository {
    constructor(repository, globalState) {
        this.repository = repository;
        this._onDidChangeRepository = new vscode_1.EventEmitter();
        this.onDidChangeRepository = this._onDidChangeRepository.event;
        this._onDidChangeState = new vscode_1.EventEmitter();
        this.onDidChangeState = this._onDidChangeState.event;
        this._onDidChangeStatus = new vscode_1.EventEmitter();
        this.onDidRunGitStatus = this._onDidChangeStatus.event;
        this._onDidChangeOriginalResource = new vscode_1.EventEmitter();
        this.onDidChangeOriginalResource = this._onDidChangeOriginalResource.event;
        this._onRunOperation = new vscode_1.EventEmitter();
        this.onRunOperation = this._onRunOperation.event;
        this._onDidRunOperation = new vscode_1.EventEmitter();
        this.onDidRunOperation = this._onDidRunOperation.event;
        this._refs = [];
        this._remotes = [];
        this._submodules = [];
        this._rebaseCommit = undefined;
        this._operations = new OperationsImpl();
        this._state = 0 /* Idle */;
        this.isRepositoryHuge = false;
        this.didWarnAboutLimit = false;
        this.isFreshRepository = undefined;
        this.disposables = [];
        const fsWatcher = vscode_1.workspace.createFileSystemWatcher('**');
        this.disposables.push(fsWatcher);
        const workspaceFilter = (uri) => util_1.isDescendant(repository.root, uri.fsPath);
        const onWorkspaceDelete = util_1.filterEvent(fsWatcher.onDidDelete, workspaceFilter);
        const onWorkspaceChange = util_1.filterEvent(util_1.anyEvent(fsWatcher.onDidChange, fsWatcher.onDidCreate), workspaceFilter);
        const onRepositoryDotGitDelete = util_1.filterEvent(onWorkspaceDelete, uri => /\/\.git$/.test(uri.path));
        const onRepositoryChange = util_1.anyEvent(onWorkspaceDelete, onWorkspaceChange);
        // relevant repository changes are:
        //  - DELETE .git folder
        //  - ANY CHANGE within .git folder except .git itself and .git/index.lock
        const onRelevantRepositoryChange = util_1.anyEvent(onRepositoryDotGitDelete, util_1.filterEvent(onRepositoryChange, uri => !/\/\.git(\/index\.lock)?$/.test(uri.path)));
        onRelevantRepositoryChange(this.onFSChange, this, this.disposables);
        const onRelevantGitChange = util_1.filterEvent(onRelevantRepositoryChange, uri => /\/\.git\//.test(uri.path));
        onRelevantGitChange(this._onDidChangeRepository.fire, this._onDidChangeRepository, this.disposables);
        const root = vscode_1.Uri.file(repository.root);
        this._sourceControl = vscode_1.scm.createSourceControl('git', 'Git', root);
        this._sourceControl.inputBox.placeholder = localize('commitMessage', "Message (press {0} to commit)");
        this._sourceControl.acceptInputCommand = { command: 'git.commitWithInput', title: localize('commit', "Commit"), arguments: [this._sourceControl] };
        this._sourceControl.quickDiffProvider = this;
        this._sourceControl.inputBox.validateInput = this.validateInput.bind(this);
        this.disposables.push(this._sourceControl);
        this._mergeGroup = this._sourceControl.createResourceGroup('merge', localize('merge changes', "Merge Changes"));
        this._indexGroup = this._sourceControl.createResourceGroup('index', localize('staged changes', "Staged Changes"));
        this._workingTreeGroup = this._sourceControl.createResourceGroup('workingTree', localize('changes', "Changes"));
        const updateIndexGroupVisibility = () => {
            const config = vscode_1.workspace.getConfiguration('git', root);
            this.indexGroup.hideWhenEmpty = !config.get('alwaysShowStagedChangesResourceGroup');
        };
        const onConfigListener = util_1.filterEvent(vscode_1.workspace.onDidChangeConfiguration, e => e.affectsConfiguration('git.alwaysShowStagedChangesResourceGroup', root));
        onConfigListener(updateIndexGroupVisibility, this, this.disposables);
        updateIndexGroupVisibility();
        this.mergeGroup.hideWhenEmpty = true;
        this.disposables.push(this.mergeGroup);
        this.disposables.push(this.indexGroup);
        this.disposables.push(this.workingTreeGroup);
        this.disposables.push(new autofetch_1.AutoFetcher(this, globalState));
        // https://github.com/Microsoft/vscode/issues/39039
        const onSuccessfulPush = util_1.filterEvent(this.onDidRunOperation, e => e.operation === "Push" /* Push */ && !e.error);
        onSuccessfulPush(() => {
            const gitConfig = vscode_1.workspace.getConfiguration('git');
            if (gitConfig.get('showPushSuccessNotification')) {
                vscode_1.window.showInformationMessage(localize('push success', "Successfully pushed."));
            }
        }, null, this.disposables);
        const statusBar = new statusbar_1.StatusBarCommands(this);
        this.disposables.push(statusBar);
        statusBar.onDidChange(() => this._sourceControl.statusBarCommands = statusBar.commands, null, this.disposables);
        this._sourceControl.statusBarCommands = statusBar.commands;
        const progressManager = new ProgressManager(this);
        this.disposables.push(progressManager);
        this.updateCommitTemplate();
        this.status();
    }
    get onDidChangeOperations() {
        return util_1.anyEvent(this.onRunOperation, this.onDidRunOperation);
    }
    get sourceControl() { return this._sourceControl; }
    get inputBox() { return this._sourceControl.inputBox; }
    get mergeGroup() { return this._mergeGroup; }
    get indexGroup() { return this._indexGroup; }
    get workingTreeGroup() { return this._workingTreeGroup; }
    get HEAD() {
        return this._HEAD;
    }
    get refs() {
        return this._refs;
    }
    get remotes() {
        return this._remotes;
    }
    get submodules() {
        return this._submodules;
    }
    set rebaseCommit(rebaseCommit) {
        if (this._rebaseCommit && !rebaseCommit) {
            this.inputBox.value = '';
        }
        else if (rebaseCommit && (!this._rebaseCommit || this._rebaseCommit.hash !== rebaseCommit.hash)) {
            this.inputBox.value = rebaseCommit.message;
        }
        this._rebaseCommit = rebaseCommit;
    }
    get rebaseCommit() {
        return this._rebaseCommit;
    }
    get operations() { return this._operations; }
    get state() { return this._state; }
    set state(state) {
        this._state = state;
        this._onDidChangeState.fire(state);
        this._HEAD = undefined;
        this._refs = [];
        this._remotes = [];
        this.mergeGroup.resourceStates = [];
        this.indexGroup.resourceStates = [];
        this.workingTreeGroup.resourceStates = [];
        this._sourceControl.count = 0;
    }
    get root() {
        return this.repository.root;
    }
    validateInput(text, position) {
        if (this.rebaseCommit) {
            if (this.rebaseCommit.message !== text) {
                return {
                    message: localize('commit in rebase', "It's not possible to change the commit message in the middle of a rebase. Please complete the rebase operation and use interactive rebase instead."),
                    type: vscode_1.SourceControlInputBoxValidationType.Warning
                };
            }
        }
        const config = vscode_1.workspace.getConfiguration('git');
        const setting = config.get('inputValidation');
        if (setting === 'off') {
            return;
        }
        if (/^\s+$/.test(text)) {
            return {
                message: localize('commitMessageWhitespacesOnlyWarning', "Current commit message only contains whitespace characters"),
                type: vscode_1.SourceControlInputBoxValidationType.Warning
            };
        }
        let lineNumber = 0;
        let start = 0, end;
        let match;
        const regex = /\r?\n/g;
        while ((match = regex.exec(text)) && position > match.index) {
            start = match.index + match[0].length;
            lineNumber++;
        }
        end = match ? match.index : text.length;
        const line = text.substring(start, end);
        let threshold = config.get('inputValidationLength', 50);
        if (lineNumber === 0) {
            const inputValidationSubjectLength = config.get('inputValidationSubjectLength', null);
            if (inputValidationSubjectLength !== null) {
                threshold = inputValidationSubjectLength;
            }
        }
        if (line.length <= threshold) {
            if (setting !== 'always') {
                return;
            }
            return {
                message: localize('commitMessageCountdown', "{0} characters left in current line", threshold - line.length),
                type: vscode_1.SourceControlInputBoxValidationType.Information
            };
        }
        else {
            return {
                message: localize('commitMessageWarning', "{0} characters over {1} in current line", line.length - threshold, threshold),
                type: vscode_1.SourceControlInputBoxValidationType.Warning
            };
        }
    }
    provideOriginalResource(uri) {
        if (uri.scheme !== 'file') {
            return;
        }
        return uri_1.toGitUri(uri, '', { replaceFileExtension: true });
    }
    async updateCommitTemplate() {
        try {
            this._sourceControl.commitTemplate = await this.repository.getCommitTemplate();
        }
        catch (e) {
            // noop
        }
    }
    getConfigs() {
        return this.run("Config" /* Config */, () => this.repository.getConfigs('local'));
    }
    getConfig(key) {
        return this.run("Config" /* Config */, () => this.repository.config('local', key));
    }
    getGlobalConfig(key) {
        return this.run("Config" /* Config */, () => this.repository.config('global', key));
    }
    setConfig(key, value) {
        return this.run("Config" /* Config */, () => this.repository.config('local', key, value));
    }
    log(options) {
        return this.run("Log" /* Log */, () => this.repository.log(options));
    }
    async status() {
        await this.run("Status" /* Status */);
    }
    diff(cached) {
        return this.run("Diff" /* Diff */, () => this.repository.diff(cached));
    }
    diffWithHEAD(path) {
        return this.run("Diff" /* Diff */, () => this.repository.diffWithHEAD(path));
    }
    diffWith(ref, path) {
        return this.run("Diff" /* Diff */, () => this.repository.diffWith(ref, path));
    }
    diffIndexWithHEAD(path) {
        return this.run("Diff" /* Diff */, () => this.repository.diffIndexWithHEAD(path));
    }
    diffIndexWith(ref, path) {
        return this.run("Diff" /* Diff */, () => this.repository.diffIndexWith(ref, path));
    }
    diffBlobs(object1, object2) {
        return this.run("Diff" /* Diff */, () => this.repository.diffBlobs(object1, object2));
    }
    diffBetween(ref1, ref2, path) {
        return this.run("Diff" /* Diff */, () => this.repository.diffBetween(ref1, ref2, path));
    }
    getMergeBase(ref1, ref2) {
        return this.run("MergeBase" /* MergeBase */, () => this.repository.getMergeBase(ref1, ref2));
    }
    async hashObject(data) {
        return this.run("HashObject" /* HashObject */, () => this.repository.hashObject(data));
    }
    async add(resources) {
        await this.run("Add" /* Add */, () => this.repository.add(resources.map(r => r.fsPath)));
    }
    async rm(resources) {
        await this.run("Remove" /* Remove */, () => this.repository.rm(resources.map(r => r.fsPath)));
    }
    async stage(resource, contents) {
        const relativePath = path.relative(this.repository.root, resource.fsPath).replace(/\\/g, '/');
        await this.run("Stage" /* Stage */, () => this.repository.stage(relativePath, contents));
        this._onDidChangeOriginalResource.fire(resource);
    }
    async revert(resources) {
        await this.run("RevertFiles" /* RevertFiles */, () => this.repository.revert('HEAD', resources.map(r => r.fsPath)));
    }
    async commit(message, opts = Object.create(null)) {
        if (this.rebaseCommit) {
            await this.run("RebaseContinue" /* RebaseContinue */, async () => {
                if (opts.all) {
                    await this.repository.add([]);
                }
                await this.repository.rebaseContinue();
            });
        }
        else {
            await this.run("Commit" /* Commit */, async () => {
                if (opts.all) {
                    await this.repository.add([]);
                }
                await this.repository.commit(message, opts);
            });
        }
    }
    async clean(resources) {
        await this.run("Clean" /* Clean */, async () => {
            const toClean = [];
            const toCheckout = [];
            const submodulesToUpdate = [];
            resources.forEach(r => {
                const fsPath = r.fsPath;
                for (const submodule of this.submodules) {
                    if (path.join(this.root, submodule.path) === fsPath) {
                        submodulesToUpdate.push(fsPath);
                        return;
                    }
                }
                const raw = r.toString();
                const scmResource = util_1.find(this.workingTreeGroup.resourceStates, sr => sr.resourceUri.toString() === raw);
                if (!scmResource) {
                    return;
                }
                switch (scmResource.type) {
                    case 7 /* UNTRACKED */:
                    case 8 /* IGNORED */:
                        toClean.push(fsPath);
                        break;
                    default:
                        toCheckout.push(fsPath);
                        break;
                }
            });
            const promises = [];
            if (toClean.length > 0) {
                promises.push(this.repository.clean(toClean));
            }
            if (toCheckout.length > 0) {
                promises.push(this.repository.checkout('', toCheckout));
            }
            if (submodulesToUpdate.length > 0) {
                promises.push(this.repository.updateSubmodules(submodulesToUpdate));
            }
            await Promise.all(promises);
        });
    }
    async branch(name, _checkout, _ref) {
        await this.run("Branch" /* Branch */, () => this.repository.branch(name, _checkout, _ref));
    }
    async deleteBranch(name, force) {
        await this.run("DeleteBranch" /* DeleteBranch */, () => this.repository.deleteBranch(name, force));
    }
    async renameBranch(name) {
        await this.run("RenameBranch" /* RenameBranch */, () => this.repository.renameBranch(name));
    }
    async getBranch(name) {
        return await this.run("GetBranch" /* GetBranch */, () => this.repository.getBranch(name));
    }
    async setBranchUpstream(name, upstream) {
        await this.run("SetBranchUpstream" /* SetBranchUpstream */, () => this.repository.setBranchUpstream(name, upstream));
    }
    async merge(ref) {
        await this.run("Merge" /* Merge */, () => this.repository.merge(ref));
    }
    async tag(name, message) {
        await this.run("Tag" /* Tag */, () => this.repository.tag(name, message));
    }
    async checkout(treeish) {
        await this.run("Checkout" /* Checkout */, () => this.repository.checkout(treeish, []));
    }
    async checkoutTracking(treeish) {
        await this.run("CheckoutTracking" /* CheckoutTracking */, () => this.repository.checkout(treeish, [], { track: true }));
    }
    async getCommit(ref) {
        return await this.repository.getCommit(ref);
    }
    async reset(treeish, hard) {
        await this.run("Reset" /* Reset */, () => this.repository.reset(treeish, hard));
    }
    async deleteRef(ref) {
        await this.run("DeleteRef" /* DeleteRef */, () => this.repository.deleteRef(ref));
    }
    async addRemote(name, url) {
        await this.run("Remote" /* Remote */, () => this.repository.addRemote(name, url));
    }
    async removeRemote(name) {
        await this.run("Remote" /* Remote */, () => this.repository.removeRemote(name));
    }
    async fetchDefault() {
        await this.run("Fetch" /* Fetch */, () => this.repository.fetch());
    }
    async fetchPrune() {
        await this.run("Fetch" /* Fetch */, () => this.repository.fetch({ prune: true }));
    }
    async fetchAll() {
        await this.run("Fetch" /* Fetch */, () => this.repository.fetch({ all: true }));
    }
    async fetch(remote, ref, depth) {
        await this.run("Fetch" /* Fetch */, () => this.repository.fetch({ remote, ref, depth }));
    }
    async pullWithRebase(head) {
        let remote;
        let branch;
        if (head && head.name && head.upstream) {
            remote = head.upstream.remote;
            branch = `${head.upstream.name}`;
        }
        return this.pullFrom(true, remote, branch);
    }
    async pull(head, unshallow) {
        let remote;
        let branch;
        if (head && head.name && head.upstream) {
            remote = head.upstream.remote;
            branch = `${head.upstream.name}`;
        }
        return this.pullFrom(false, remote, branch, unshallow);
    }
    async pullFrom(rebase, remote, branch, unshallow) {
        await this.run("Pull" /* Pull */, async () => {
            await this.maybeAutoStash(async () => {
                const config = vscode_1.workspace.getConfiguration('git', vscode_1.Uri.file(this.root));
                const fetchOnPull = config.get('fetchOnPull');
                if (fetchOnPull) {
                    await this.repository.pull(rebase, undefined, undefined, { unshallow });
                }
                else {
                    await this.repository.pull(rebase, remote, branch, { unshallow });
                }
            });
        });
    }
    async push(head, forcePushMode) {
        let remote;
        let branch;
        if (head && head.name && head.upstream) {
            remote = head.upstream.remote;
            branch = `${head.name}:${head.upstream.name}`;
        }
        await this.run("Push" /* Push */, () => this.repository.push(remote, branch, undefined, undefined, forcePushMode));
    }
    async pushTo(remote, name, setUpstream = false, forcePushMode) {
        await this.run("Push" /* Push */, () => this.repository.push(remote, name, setUpstream, undefined, forcePushMode));
    }
    async pushTags(remote, forcePushMode) {
        await this.run("Push" /* Push */, () => this.repository.push(remote, undefined, false, true, forcePushMode));
    }
    async blame(path) {
        return await this.run("Blame" /* Blame */, () => this.repository.blame(path));
    }
    sync(head) {
        return this._sync(head, false);
    }
    async syncRebase(head) {
        return this._sync(head, true);
    }
    async _sync(head, rebase) {
        let remoteName;
        let pullBranch;
        let pushBranch;
        if (head.name && head.upstream) {
            remoteName = head.upstream.remote;
            pullBranch = `${head.upstream.name}`;
            pushBranch = `${head.name}:${head.upstream.name}`;
        }
        await this.run("Sync" /* Sync */, async () => {
            await this.maybeAutoStash(async () => {
                const config = vscode_1.workspace.getConfiguration('git', vscode_1.Uri.file(this.root));
                const fetchOnPull = config.get('fetchOnPull');
                if (fetchOnPull) {
                    await this.repository.pull(rebase);
                }
                else {
                    await this.repository.pull(rebase, remoteName, pullBranch);
                }
                const remote = this.remotes.find(r => r.name === remoteName);
                if (remote && remote.isReadOnly) {
                    return;
                }
                const shouldPush = this.HEAD && (typeof this.HEAD.ahead === 'number' ? this.HEAD.ahead > 0 : true);
                if (shouldPush) {
                    await this.repository.push(remoteName, pushBranch);
                }
            });
        });
    }
    async show(ref, filePath) {
        return await this.run("Show" /* Show */, async () => {
            const relativePath = path.relative(this.repository.root, filePath).replace(/\\/g, '/');
            const configFiles = vscode_1.workspace.getConfiguration('files', vscode_1.Uri.file(filePath));
            const defaultEncoding = configFiles.get('encoding');
            const autoGuessEncoding = configFiles.get('autoGuessEncoding');
            try {
                return await this.repository.bufferString(`${ref}:${relativePath}`, defaultEncoding, autoGuessEncoding);
            }
            catch (err) {
                if (err.gitErrorCode === "WrongCase" /* WrongCase */) {
                    const gitRelativePath = await this.repository.getGitRelativePath(ref, relativePath);
                    return await this.repository.bufferString(`${ref}:${gitRelativePath}`, defaultEncoding, autoGuessEncoding);
                }
                throw err;
            }
        });
    }
    async buffer(ref, filePath) {
        return this.run("Show" /* Show */, () => {
            const relativePath = path.relative(this.repository.root, filePath).replace(/\\/g, '/');
            return this.repository.buffer(`${ref}:${relativePath}`);
        });
    }
    getObjectDetails(ref, filePath) {
        return this.run("GetObjectDetails" /* GetObjectDetails */, () => this.repository.getObjectDetails(ref, filePath));
    }
    detectObjectType(object) {
        return this.run("Show" /* Show */, () => this.repository.detectObjectType(object));
    }
    async apply(patch, reverse) {
        return await this.run("Apply" /* Apply */, () => this.repository.apply(patch, reverse));
    }
    async getStashes() {
        return await this.repository.getStashes();
    }
    async createStash(message, includeUntracked) {
        return await this.run("Stash" /* Stash */, () => this.repository.createStash(message, includeUntracked));
    }
    async popStash(index) {
        return await this.run("Stash" /* Stash */, () => this.repository.popStash(index));
    }
    async applyStash(index) {
        return await this.run("Stash" /* Stash */, () => this.repository.applyStash(index));
    }
    async getCommitTemplate() {
        return await this.run("GetCommitTemplate" /* GetCommitTemplate */, async () => this.repository.getCommitTemplate());
    }
    async ignore(files) {
        return await this.run("Ignore" /* Ignore */, async () => {
            const ignoreFile = `${this.repository.root}${path.sep}.gitignore`;
            const textToAppend = files
                .map(uri => path.relative(this.repository.root, uri.fsPath).replace(/\\/g, '/'))
                .join('\n');
            const document = await new Promise(c => fs.exists(ignoreFile, c))
                ? await vscode_1.workspace.openTextDocument(ignoreFile)
                : await vscode_1.workspace.openTextDocument(vscode_1.Uri.file(ignoreFile).with({ scheme: 'untitled' }));
            await vscode_1.window.showTextDocument(document);
            const edit = new vscode_1.WorkspaceEdit();
            const lastLine = document.lineAt(document.lineCount - 1);
            const text = lastLine.isEmptyOrWhitespace ? `${textToAppend}\n` : `\n${textToAppend}\n`;
            edit.insert(document.uri, lastLine.range.end, text);
            await vscode_1.workspace.applyEdit(edit);
            await document.save();
        });
    }
    checkIgnore(filePaths) {
        return this.run("CheckIgnore" /* CheckIgnore */, () => {
            return new Promise((resolve, reject) => {
                filePaths = filePaths
                    .filter(filePath => util_1.isDescendant(this.root, filePath));
                if (filePaths.length === 0) {
                    // nothing left
                    return resolve(new Set());
                }
                // https://git-scm.com/docs/git-check-ignore#git-check-ignore--z
                const child = this.repository.stream(['check-ignore', '-z', '--stdin'], { stdio: [null, null, null] });
                child.stdin.end(filePaths.join('\0'), 'utf8');
                const onExit = (exitCode) => {
                    if (exitCode === 1) {
                        // nothing ignored
                        resolve(new Set());
                    }
                    else if (exitCode === 0) {
                        // paths are separated by the null-character
                        resolve(new Set(data.split('\0')));
                    }
                    else {
                        if (/ is in submodule /.test(stderr)) {
                            reject(new git_1.GitError({ stdout: data, stderr, exitCode, gitErrorCode: "IsInSubmodule" /* IsInSubmodule */ }));
                        }
                        else {
                            reject(new git_1.GitError({ stdout: data, stderr, exitCode }));
                        }
                    }
                };
                let data = '';
                const onStdoutData = (raw) => {
                    data += raw;
                };
                child.stdout.setEncoding('utf8');
                child.stdout.on('data', onStdoutData);
                let stderr = '';
                child.stderr.setEncoding('utf8');
                child.stderr.on('data', raw => stderr += raw);
                child.on('error', reject);
                child.on('exit', onExit);
            });
        });
    }
    async run(operation, runOperation = () => Promise.resolve(null)) {
        if (this.state !== 0 /* Idle */) {
            throw new Error('Repository not initialized');
        }
        let error = null;
        this._operations.start(operation);
        this._onRunOperation.fire(operation);
        try {
            const result = await this.retryRun(operation, runOperation);
            if (!isReadOnly(operation)) {
                await this.updateModelState();
            }
            return result;
        }
        catch (err) {
            error = err;
            if (err.gitErrorCode === "NotAGitRepository" /* NotAGitRepository */) {
                this.state = 1 /* Disposed */;
            }
            throw err;
        }
        finally {
            this._operations.end(operation);
            this._onDidRunOperation.fire({ operation, error });
        }
    }
    async retryRun(operation, runOperation = () => Promise.resolve(null)) {
        let attempt = 0;
        while (true) {
            try {
                attempt++;
                return await runOperation();
            }
            catch (err) {
                const shouldRetry = attempt <= 10 && ((err.gitErrorCode === "RepositoryIsLocked" /* RepositoryIsLocked */)
                    || ((operation === "Pull" /* Pull */ || operation === "Sync" /* Sync */ || operation === "Fetch" /* Fetch */) && (err.gitErrorCode === "CantLockRef" /* CantLockRef */ || err.gitErrorCode === "CantRebaseMultipleBranches" /* CantRebaseMultipleBranches */)));
                if (shouldRetry) {
                    // quatratic backoff
                    await timeout(Math.pow(attempt, 2) * 50);
                }
                else {
                    throw err;
                }
            }
        }
    }
    async findKnownHugeFolderPathsToIgnore() {
        const folderPaths = [];
        for (const folderName of Repository.KnownHugeFolderNames) {
            const folderPath = path.join(this.repository.root, folderName);
            if (await new Promise(c => fs.exists(folderPath, c))) {
                folderPaths.push(folderPath);
            }
        }
        const ignored = await this.checkIgnore(folderPaths);
        return folderPaths.filter(p => !ignored.has(p));
    }
    async updateModelState() {
        const { status, didHitLimit } = await this.repository.getStatus();
        const config = vscode_1.workspace.getConfiguration('git');
        const shouldIgnore = config.get('ignoreLimitWarning') === true;
        const useIcons = !config.get('decorations.enabled', true);
        this.isRepositoryHuge = didHitLimit;
        if (didHitLimit && !shouldIgnore && !this.didWarnAboutLimit) {
            const knownHugeFolderPaths = await this.findKnownHugeFolderPathsToIgnore();
            const gitWarn = localize('huge', "The git repository at '{0}' has too many active changes, only a subset of Git features will be enabled.", this.repository.root);
            const neverAgain = { title: localize('neveragain', "Don't Show Again") };
            if (knownHugeFolderPaths.length > 0) {
                const folderPath = knownHugeFolderPaths[0];
                const folderName = path.basename(folderPath);
                const addKnown = localize('add known', "Would you like to add '{0}' to .gitignore?", folderName);
                const yes = { title: localize('yes', "Yes") };
                const result = await vscode_1.window.showWarningMessage(`${gitWarn} ${addKnown}`, yes, neverAgain);
                if (result === neverAgain) {
                    config.update('ignoreLimitWarning', true, false);
                    this.didWarnAboutLimit = true;
                }
                else if (result === yes) {
                    this.ignore([vscode_1.Uri.file(folderPath)]);
                }
            }
            else {
                const result = await vscode_1.window.showWarningMessage(gitWarn, neverAgain);
                if (result === neverAgain) {
                    config.update('ignoreLimitWarning', true, false);
                }
                this.didWarnAboutLimit = true;
            }
        }
        let HEAD;
        try {
            HEAD = await this.repository.getHEAD();
            if (HEAD.name) {
                try {
                    HEAD = await this.repository.getBranch(HEAD.name);
                }
                catch (err) {
                    // noop
                }
            }
        }
        catch (err) {
            // noop
        }
        const [refs, remotes, submodules, rebaseCommit] = await Promise.all([this.repository.getRefs(), this.repository.getRemotes(), this.repository.getSubmodules(), this.getRebaseCommit()]);
        this._HEAD = HEAD;
        this._refs = refs;
        this._remotes = remotes;
        this._submodules = submodules;
        this.rebaseCommit = rebaseCommit;
        const index = [];
        const workingTree = [];
        const merge = [];
        status.forEach(raw => {
            const uri = vscode_1.Uri.file(path.join(this.repository.root, raw.path));
            const renameUri = raw.rename ? vscode_1.Uri.file(path.join(this.repository.root, raw.rename)) : undefined;
            switch (raw.x + raw.y) {
                case '??': return workingTree.push(new Resource(2 /* WorkingTree */, uri, 7 /* UNTRACKED */, useIcons));
                case '!!': return workingTree.push(new Resource(2 /* WorkingTree */, uri, 8 /* IGNORED */, useIcons));
                case 'DD': return merge.push(new Resource(0 /* Merge */, uri, 15 /* BOTH_DELETED */, useIcons));
                case 'AU': return merge.push(new Resource(0 /* Merge */, uri, 10 /* ADDED_BY_US */, useIcons));
                case 'UD': return merge.push(new Resource(0 /* Merge */, uri, 13 /* DELETED_BY_THEM */, useIcons));
                case 'UA': return merge.push(new Resource(0 /* Merge */, uri, 11 /* ADDED_BY_THEM */, useIcons));
                case 'DU': return merge.push(new Resource(0 /* Merge */, uri, 12 /* DELETED_BY_US */, useIcons));
                case 'AA': return merge.push(new Resource(0 /* Merge */, uri, 14 /* BOTH_ADDED */, useIcons));
                case 'UU': return merge.push(new Resource(0 /* Merge */, uri, 16 /* BOTH_MODIFIED */, useIcons));
            }
            switch (raw.x) {
                case 'M':
                    index.push(new Resource(1 /* Index */, uri, 0 /* INDEX_MODIFIED */, useIcons));
                    break;
                case 'A':
                    index.push(new Resource(1 /* Index */, uri, 1 /* INDEX_ADDED */, useIcons));
                    break;
                case 'D':
                    index.push(new Resource(1 /* Index */, uri, 2 /* INDEX_DELETED */, useIcons));
                    break;
                case 'R':
                    index.push(new Resource(1 /* Index */, uri, 3 /* INDEX_RENAMED */, useIcons, renameUri));
                    break;
                case 'C':
                    index.push(new Resource(1 /* Index */, uri, 4 /* INDEX_COPIED */, useIcons, renameUri));
                    break;
            }
            switch (raw.y) {
                case 'M':
                    workingTree.push(new Resource(2 /* WorkingTree */, uri, 5 /* MODIFIED */, useIcons, renameUri));
                    break;
                case 'D':
                    workingTree.push(new Resource(2 /* WorkingTree */, uri, 6 /* DELETED */, useIcons, renameUri));
                    break;
                case 'A':
                    workingTree.push(new Resource(2 /* WorkingTree */, uri, 9 /* INTENT_TO_ADD */, useIcons, renameUri));
                    break;
            }
            return undefined;
        });
        // set resource groups
        this.mergeGroup.resourceStates = merge;
        this.indexGroup.resourceStates = index;
        this.workingTreeGroup.resourceStates = workingTree;
        // set count badge
        const countBadge = vscode_1.workspace.getConfiguration('git').get('countBadge');
        let count = merge.length + index.length + workingTree.length;
        switch (countBadge) {
            case 'off':
                count = 0;
                break;
            case 'tracked':
                count = count - workingTree.filter(r => r.type === 7 /* UNTRACKED */ || r.type === 8 /* IGNORED */).length;
                break;
        }
        this._sourceControl.count = count;
        // Disable `Discard All Changes` for "fresh" repositories
        // https://github.com/Microsoft/vscode/issues/43066
        const isFreshRepository = !this._HEAD || !this._HEAD.commit;
        if (this.isFreshRepository !== isFreshRepository) {
            vscode_1.commands.executeCommand('setContext', 'gitFreshRepository', isFreshRepository);
            this.isFreshRepository = isFreshRepository;
        }
        this._onDidChangeStatus.fire();
    }
    async getRebaseCommit() {
        const rebaseHeadPath = path.join(this.repository.root, '.git', 'REBASE_HEAD');
        const rebaseApplyPath = path.join(this.repository.root, '.git', 'rebase-apply');
        const rebaseMergePath = path.join(this.repository.root, '.git', 'rebase-merge');
        try {
            const [rebaseApplyExists, rebaseMergePathExists, rebaseHead] = await Promise.all([
                new Promise(c => fs.exists(rebaseApplyPath, c)),
                new Promise(c => fs.exists(rebaseMergePath, c)),
                new Promise((c, e) => fs.readFile(rebaseHeadPath, 'utf8', (err, result) => err ? e(err) : c(result)))
            ]);
            if (!rebaseApplyExists && !rebaseMergePathExists) {
                return undefined;
            }
            return await this.getCommit(rebaseHead.trim());
        }
        catch (err) {
            return undefined;
        }
    }
    async maybeAutoStash(runOperation) {
        const config = vscode_1.workspace.getConfiguration('git', vscode_1.Uri.file(this.root));
        const shouldAutoStash = config.get('autoStash')
            && this.workingTreeGroup.resourceStates.some(r => r.type !== 7 /* UNTRACKED */ && r.type !== 8 /* IGNORED */);
        if (!shouldAutoStash) {
            return await runOperation();
        }
        await this.repository.createStash(undefined, true);
        const result = await runOperation();
        await this.repository.popStash();
        return result;
    }
    onFSChange(_uri) {
        const config = vscode_1.workspace.getConfiguration('git');
        const autorefresh = config.get('autorefresh');
        if (!autorefresh) {
            return;
        }
        if (this.isRepositoryHuge) {
            return;
        }
        if (!this.operations.isIdle()) {
            return;
        }
        this.eventuallyUpdateWhenIdleAndWait();
    }
    eventuallyUpdateWhenIdleAndWait() {
        this.updateWhenIdleAndWait();
    }
    async updateWhenIdleAndWait() {
        await this.whenIdleAndFocused();
        await this.status();
        await timeout(5000);
    }
    async whenIdleAndFocused() {
        while (true) {
            if (!this.operations.isIdle()) {
                await util_1.eventToPromise(this.onDidRunOperation);
                continue;
            }
            if (!vscode_1.window.state.focused) {
                const onDidFocusWindow = util_1.filterEvent(vscode_1.window.onDidChangeWindowState, e => e.focused);
                await util_1.eventToPromise(onDidFocusWindow);
                continue;
            }
            return;
        }
    }
    get headLabel() {
        const HEAD = this.HEAD;
        if (!HEAD) {
            return '';
        }
        const tag = this.refs.filter(iref => iref.type === 2 /* Tag */ && iref.commit === HEAD.commit)[0];
        const tagName = tag && tag.name;
        const head = HEAD.name || tagName || (HEAD.commit || '').substr(0, 8);
        return head
            + (this.workingTreeGroup.resourceStates.length > 0 ? '*' : '')
            + (this.indexGroup.resourceStates.length > 0 ? '+' : '')
            + (this.mergeGroup.resourceStates.length > 0 ? '!' : '');
    }
    get syncLabel() {
        if (!this.HEAD
            || !this.HEAD.name
            || !this.HEAD.commit
            || !this.HEAD.upstream
            || !(this.HEAD.ahead || this.HEAD.behind)) {
            return '';
        }
        const remoteName = this.HEAD && this.HEAD.remote || this.HEAD.upstream.remote;
        const remote = this.remotes.find(r => r.name === remoteName);
        if (remote && remote.isReadOnly) {
            return `${this.HEAD.behind}↓`;
        }
        return `${this.HEAD.behind}↓ ${this.HEAD.ahead}↑`;
    }
    dispose() {
        this.disposables = util_1.dispose(this.disposables);
    }
}
Repository.KnownHugeFolderNames = ['node_modules'];
__decorate([
    decorators_1.memoize
], Repository.prototype, "onDidChangeOperations", null);
__decorate([
    decorators_1.throttle
], Repository.prototype, "status", null);
__decorate([
    decorators_1.throttle
], Repository.prototype, "fetchDefault", null);
__decorate([
    decorators_1.throttle
], Repository.prototype, "fetchPrune", null);
__decorate([
    decorators_1.throttle
], Repository.prototype, "fetchAll", null);
__decorate([
    decorators_1.throttle
], Repository.prototype, "pullWithRebase", null);
__decorate([
    decorators_1.throttle
], Repository.prototype, "pull", null);
__decorate([
    decorators_1.throttle
], Repository.prototype, "push", null);
__decorate([
    decorators_1.throttle
], Repository.prototype, "sync", null);
__decorate([
    decorators_1.throttle
], Repository.prototype, "syncRebase", null);
__decorate([
    decorators_1.throttle
], Repository.prototype, "updateModelState", null);
__decorate([
    decorators_1.debounce(1000)
], Repository.prototype, "eventuallyUpdateWhenIdleAndWait", null);
__decorate([
    decorators_1.throttle
], Repository.prototype, "updateWhenIdleAndWait", null);
exports.Repository = Repository;
//# sourceMappingURL=repository.js.map