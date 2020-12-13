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
exports.Model = void 0;
const vscode_1 = require("vscode");
const repository_1 = require("./repository");
const decorators_1 = require("./decorators");
const util_1 = require("./util");
const path = require("path");
const fs = require("fs");
const nls = require("vscode-nls");
const uri_1 = require("./uri");
const localize = nls.loadMessageBundle();
class RepositoryPick {
    constructor(repository, index) {
        this.repository = repository;
        this.index = index;
    }
    get label() {
        return path.basename(this.repository.root);
    }
    get description() {
        return [this.repository.headLabel, this.repository.syncLabel]
            .filter(l => !!l)
            .join(' ');
    }
}
__decorate([
    decorators_1.memoize
], RepositoryPick.prototype, "label", null);
__decorate([
    decorators_1.memoize
], RepositoryPick.prototype, "description", null);
class Model {
    constructor(git, askpass, globalState, outputChannel) {
        this.git = git;
        this.askpass = askpass;
        this.globalState = globalState;
        this.outputChannel = outputChannel;
        this._onDidOpenRepository = new vscode_1.EventEmitter();
        this.onDidOpenRepository = this._onDidOpenRepository.event;
        this._onDidCloseRepository = new vscode_1.EventEmitter();
        this.onDidCloseRepository = this._onDidCloseRepository.event;
        this._onDidChangeRepository = new vscode_1.EventEmitter();
        this.onDidChangeRepository = this._onDidChangeRepository.event;
        this._onDidChangeOriginalResource = new vscode_1.EventEmitter();
        this.onDidChangeOriginalResource = this._onDidChangeOriginalResource.event;
        this.openRepositories = [];
        this.possibleGitRepositoryPaths = new Set();
        this._onDidChangeState = new vscode_1.EventEmitter();
        this.onDidChangeState = this._onDidChangeState.event;
        this._state = 'uninitialized';
        this.remoteSourceProviders = new Set();
        this._onDidAddRemoteSourceProvider = new vscode_1.EventEmitter();
        this.onDidAddRemoteSourceProvider = this._onDidAddRemoteSourceProvider.event;
        this._onDidRemoveRemoteSourceProvider = new vscode_1.EventEmitter();
        this.onDidRemoveRemoteSourceProvider = this._onDidRemoveRemoteSourceProvider.event;
        this.pushErrorHandlers = new Set();
        this.disposables = [];
        vscode_1.workspace.onDidChangeWorkspaceFolders(this.onDidChangeWorkspaceFolders, this, this.disposables);
        vscode_1.window.onDidChangeVisibleTextEditors(this.onDidChangeVisibleTextEditors, this, this.disposables);
        vscode_1.workspace.onDidChangeConfiguration(this.onDidChangeConfiguration, this, this.disposables);
        const fsWatcher = vscode_1.workspace.createFileSystemWatcher('**');
        this.disposables.push(fsWatcher);
        const onWorkspaceChange = util_1.anyEvent(fsWatcher.onDidChange, fsWatcher.onDidCreate, fsWatcher.onDidDelete);
        const onGitRepositoryChange = util_1.filterEvent(onWorkspaceChange, uri => /\/\.git/.test(uri.path));
        const onPossibleGitRepositoryChange = util_1.filterEvent(onGitRepositoryChange, uri => !this.getRepository(uri));
        onPossibleGitRepositoryChange(this.onPossibleGitRepositoryChange, this, this.disposables);
        this.setState('uninitialized');
        this.doInitialScan().finally(() => this.setState('initialized'));
    }
    get repositories() { return this.openRepositories.map(r => r.repository); }
    get state() { return this._state; }
    setState(state) {
        this._state = state;
        this._onDidChangeState.fire(state);
        vscode_1.commands.executeCommand('setContext', 'git.state', state);
    }
    get isInitialized() {
        if (this._state === 'initialized') {
            return Promise.resolve();
        }
        return util_1.eventToPromise(util_1.filterEvent(this.onDidChangeState, s => s === 'initialized'));
    }
    async doInitialScan() {
        await Promise.all([
            this.onDidChangeWorkspaceFolders({ added: vscode_1.workspace.workspaceFolders || [], removed: [] }),
            this.onDidChangeVisibleTextEditors(vscode_1.window.visibleTextEditors),
            this.scanWorkspaceFolders()
        ]);
    }
    /**
     * Scans the first level of each workspace folder, looking
     * for git repositories.
     */
    async scanWorkspaceFolders() {
        const config = vscode_1.workspace.getConfiguration('git');
        const autoRepositoryDetection = config.get('autoRepositoryDetection');
        if (autoRepositoryDetection !== true && autoRepositoryDetection !== 'subFolders') {
            return;
        }
        await Promise.all((vscode_1.workspace.workspaceFolders || []).map(async (folder) => {
            const root = folder.uri.fsPath;
            const children = await new Promise((c, e) => fs.readdir(root, (err, r) => err ? e(err) : c(r)));
            const promises = children
                .filter(child => child !== '.git')
                .map(child => this.openRepository(path.join(root, child)));
            const folderConfig = vscode_1.workspace.getConfiguration('git', folder.uri);
            const paths = folderConfig.get('scanRepositories') || [];
            for (const possibleRepositoryPath of paths) {
                if (path.isAbsolute(possibleRepositoryPath)) {
                    console.warn(localize('not supported', "Absolute paths not supported in 'git.scanRepositories' setting."));
                    continue;
                }
                promises.push(this.openRepository(path.join(root, possibleRepositoryPath)));
            }
            await Promise.all(promises);
        }));
    }
    onPossibleGitRepositoryChange(uri) {
        const config = vscode_1.workspace.getConfiguration('git');
        const autoRepositoryDetection = config.get('autoRepositoryDetection');
        if (autoRepositoryDetection === false) {
            return;
        }
        this.eventuallyScanPossibleGitRepository(uri.fsPath.replace(/\.git.*$/, ''));
    }
    eventuallyScanPossibleGitRepository(path) {
        this.possibleGitRepositoryPaths.add(path);
        this.eventuallyScanPossibleGitRepositories();
    }
    eventuallyScanPossibleGitRepositories() {
        for (const path of this.possibleGitRepositoryPaths) {
            this.openRepository(path);
        }
        this.possibleGitRepositoryPaths.clear();
    }
    async onDidChangeWorkspaceFolders({ added, removed }) {
        const possibleRepositoryFolders = added
            .filter(folder => !this.getOpenRepository(folder.uri));
        const activeRepositoriesList = vscode_1.window.visibleTextEditors
            .map(editor => this.getRepository(editor.document.uri))
            .filter(repository => !!repository);
        const activeRepositories = new Set(activeRepositoriesList);
        const openRepositoriesToDispose = removed
            .map(folder => this.getOpenRepository(folder.uri))
            .filter(r => !!r)
            .filter(r => !activeRepositories.has(r.repository))
            .filter(r => !(vscode_1.workspace.workspaceFolders || []).some(f => util_1.isDescendant(f.uri.fsPath, r.repository.root)));
        openRepositoriesToDispose.forEach(r => r.dispose());
        await Promise.all(possibleRepositoryFolders.map(p => this.openRepository(p.uri.fsPath)));
    }
    onDidChangeConfiguration() {
        const possibleRepositoryFolders = (vscode_1.workspace.workspaceFolders || [])
            .filter(folder => vscode_1.workspace.getConfiguration('git', folder.uri).get('enabled') === true)
            .filter(folder => !this.getOpenRepository(folder.uri));
        const openRepositoriesToDispose = this.openRepositories
            .map(repository => ({ repository, root: vscode_1.Uri.file(repository.repository.root) }))
            .filter(({ root }) => vscode_1.workspace.getConfiguration('git', root).get('enabled') !== true)
            .map(({ repository }) => repository);
        possibleRepositoryFolders.forEach(p => this.openRepository(p.uri.fsPath));
        openRepositoriesToDispose.forEach(r => r.dispose());
    }
    async onDidChangeVisibleTextEditors(editors) {
        const config = vscode_1.workspace.getConfiguration('git');
        const autoRepositoryDetection = config.get('autoRepositoryDetection');
        if (autoRepositoryDetection !== true && autoRepositoryDetection !== 'openEditors') {
            return;
        }
        await Promise.all(editors.map(async (editor) => {
            const uri = editor.document.uri;
            if (uri.scheme !== 'file') {
                return;
            }
            const repository = this.getRepository(uri);
            if (repository) {
                return;
            }
            await this.openRepository(path.dirname(uri.fsPath));
        }));
    }
    async openRepository(path) {
        if (this.getRepository(path)) {
            return;
        }
        const config = vscode_1.workspace.getConfiguration('git', vscode_1.Uri.file(path));
        const enabled = config.get('enabled') === true;
        if (!enabled) {
            return;
        }
        try {
            const rawRoot = await this.git.getRepositoryRoot(path);
            // This can happen whenever `path` has the wrong case sensitivity in
            // case insensitive file systems
            // https://github.com/microsoft/vscode/issues/33498
            const repositoryRoot = vscode_1.Uri.file(rawRoot).fsPath;
            if (this.getRepository(repositoryRoot)) {
                return;
            }
            if (this.shouldRepositoryBeIgnored(rawRoot)) {
                return;
            }
            const dotGit = await this.git.getRepositoryDotGit(repositoryRoot);
            const repository = new repository_1.Repository(this.git.open(repositoryRoot, dotGit), this, this, this.globalState, this.outputChannel);
            this.open(repository);
            await repository.status();
        }
        catch (err) {
            // noop
        }
    }
    shouldRepositoryBeIgnored(repositoryRoot) {
        const config = vscode_1.workspace.getConfiguration('git');
        const ignoredRepos = config.get('ignoredRepositories') || [];
        for (const ignoredRepo of ignoredRepos) {
            if (path.isAbsolute(ignoredRepo)) {
                if (util_1.pathEquals(ignoredRepo, repositoryRoot)) {
                    return true;
                }
            }
            else {
                for (const folder of vscode_1.workspace.workspaceFolders || []) {
                    if (util_1.pathEquals(path.join(folder.uri.fsPath, ignoredRepo), repositoryRoot)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    open(repository) {
        this.outputChannel.appendLine(`Open repository: ${repository.root}`);
        const onDidDisappearRepository = util_1.filterEvent(repository.onDidChangeState, state => state === 1 /* Disposed */);
        const disappearListener = onDidDisappearRepository(() => dispose());
        const changeListener = repository.onDidChangeRepository(uri => this._onDidChangeRepository.fire({ repository, uri }));
        const originalResourceChangeListener = repository.onDidChangeOriginalResource(uri => this._onDidChangeOriginalResource.fire({ repository, uri }));
        const shouldDetectSubmodules = vscode_1.workspace
            .getConfiguration('git', vscode_1.Uri.file(repository.root))
            .get('detectSubmodules');
        const submodulesLimit = vscode_1.workspace
            .getConfiguration('git', vscode_1.Uri.file(repository.root))
            .get('detectSubmodulesLimit');
        const checkForSubmodules = () => {
            if (!shouldDetectSubmodules) {
                return;
            }
            if (repository.submodules.length > submodulesLimit) {
                vscode_1.window.showWarningMessage(localize('too many submodules', "The '{0}' repository has {1} submodules which won't be opened automatically. You can still open each one individually by opening a file within.", path.basename(repository.root), repository.submodules.length));
                statusListener.dispose();
            }
            repository.submodules
                .slice(0, submodulesLimit)
                .map(r => path.join(repository.root, r.path))
                .forEach(p => this.eventuallyScanPossibleGitRepository(p));
        };
        const statusListener = repository.onDidRunGitStatus(checkForSubmodules);
        checkForSubmodules();
        const dispose = () => {
            disappearListener.dispose();
            changeListener.dispose();
            originalResourceChangeListener.dispose();
            statusListener.dispose();
            repository.dispose();
            this.openRepositories = this.openRepositories.filter(e => e !== openRepository);
            this._onDidCloseRepository.fire(repository);
        };
        const openRepository = { repository, dispose };
        this.openRepositories.push(openRepository);
        this._onDidOpenRepository.fire(repository);
    }
    close(repository) {
        const openRepository = this.getOpenRepository(repository);
        if (!openRepository) {
            return;
        }
        this.outputChannel.appendLine(`Close repository: ${repository.root}`);
        openRepository.dispose();
    }
    async pickRepository() {
        if (this.openRepositories.length === 0) {
            throw new Error(localize('no repositories', "There are no available repositories"));
        }
        const picks = this.openRepositories.map((e, index) => new RepositoryPick(e.repository, index));
        const active = vscode_1.window.activeTextEditor;
        const repository = active && this.getRepository(active.document.fileName);
        const index = picks.findIndex(pick => pick.repository === repository);
        // Move repository pick containing the active text editor to appear first
        if (index > -1) {
            picks.unshift(...picks.splice(index, 1));
        }
        const placeHolder = localize('pick repo', "Choose a repository");
        const pick = await vscode_1.window.showQuickPick(picks, { placeHolder });
        return pick && pick.repository;
    }
    getRepository(hint) {
        const liveRepository = this.getOpenRepository(hint);
        return liveRepository && liveRepository.repository;
    }
    getOpenRepository(hint) {
        if (!hint) {
            return undefined;
        }
        if (hint instanceof repository_1.Repository) {
            return this.openRepositories.filter(r => r.repository === hint)[0];
        }
        if (typeof hint === 'string') {
            hint = vscode_1.Uri.file(hint);
        }
        if (hint instanceof vscode_1.Uri) {
            let resourcePath;
            if (hint.scheme === 'git') {
                resourcePath = uri_1.fromGitUri(hint).path;
            }
            else {
                resourcePath = hint.fsPath;
            }
            outer: for (const liveRepository of this.openRepositories.sort((a, b) => b.repository.root.length - a.repository.root.length)) {
                if (!util_1.isDescendant(liveRepository.repository.root, resourcePath)) {
                    continue;
                }
                for (const submodule of liveRepository.repository.submodules) {
                    const submoduleRoot = path.join(liveRepository.repository.root, submodule.path);
                    if (util_1.isDescendant(submoduleRoot, resourcePath)) {
                        continue outer;
                    }
                }
                return liveRepository;
            }
            return undefined;
        }
        for (const liveRepository of this.openRepositories) {
            const repository = liveRepository.repository;
            if (hint === repository.sourceControl) {
                return liveRepository;
            }
            if (hint === repository.mergeGroup || hint === repository.indexGroup || hint === repository.workingTreeGroup) {
                return liveRepository;
            }
        }
        return undefined;
    }
    getRepositoryForSubmodule(submoduleUri) {
        for (const repository of this.repositories) {
            for (const submodule of repository.submodules) {
                const submodulePath = path.join(repository.root, submodule.path);
                if (submodulePath === submoduleUri.fsPath) {
                    return repository;
                }
            }
        }
        return undefined;
    }
    registerRemoteSourceProvider(provider) {
        this.remoteSourceProviders.add(provider);
        this._onDidAddRemoteSourceProvider.fire(provider);
        return util_1.toDisposable(() => {
            this.remoteSourceProviders.delete(provider);
            this._onDidRemoveRemoteSourceProvider.fire(provider);
        });
    }
    registerCredentialsProvider(provider) {
        return this.askpass.registerCredentialsProvider(provider);
    }
    getRemoteProviders() {
        return [...this.remoteSourceProviders.values()];
    }
    registerPushErrorHandler(handler) {
        this.pushErrorHandlers.add(handler);
        return util_1.toDisposable(() => this.pushErrorHandlers.delete(handler));
    }
    getPushErrorHandlers() {
        return [...this.pushErrorHandlers];
    }
    dispose() {
        const openRepositories = [...this.openRepositories];
        openRepositories.forEach(r => r.dispose());
        this.openRepositories = [];
        this.possibleGitRepositoryPaths.clear();
        this.disposables = util_1.dispose(this.disposables);
    }
}
__decorate([
    decorators_1.memoize
], Model.prototype, "isInitialized", null);
__decorate([
    decorators_1.debounce(500)
], Model.prototype, "eventuallyScanPossibleGitRepositories", null);
__decorate([
    decorators_1.sequentialize
], Model.prototype, "openRepository", null);
exports.Model = Model;
//# sourceMappingURL=model.js.map