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
const repository_1 = require("./repository");
const uri_1 = require("./uri");
const util_1 = require("./util");
const staging_1 = require("./staging");
const path = require("path");
const fs_1 = require("fs");
const os = require("os");
const nls = require("vscode-nls");
const localize = nls.loadMessageBundle();
class CheckoutItem {
    constructor(ref) {
        this.ref = ref;
    }
    get shortCommit() { return (this.ref.commit || '').substr(0, 8); }
    get label() { return this.ref.name || this.shortCommit; }
    get description() { return this.shortCommit; }
    async run(repository) {
        const ref = this.ref.name;
        if (!ref) {
            return;
        }
        await repository.checkout(ref);
    }
}
class CheckoutTagItem extends CheckoutItem {
    get description() {
        return localize('tag at', "Tag at {0}", this.shortCommit);
    }
}
class CheckoutRemoteHeadItem extends CheckoutItem {
    get description() {
        return localize('remote branch at', "Remote branch at {0}", this.shortCommit);
    }
    async run(repository) {
        if (!this.ref.name) {
            return;
        }
        await repository.checkoutTracking(this.ref.name);
    }
}
class BranchDeleteItem {
    constructor(ref) {
        this.ref = ref;
    }
    get shortCommit() { return (this.ref.commit || '').substr(0, 8); }
    get branchName() { return this.ref.name; }
    get label() { return this.branchName || ''; }
    get description() { return this.shortCommit; }
    async run(repository, force) {
        if (!this.branchName) {
            return;
        }
        await repository.deleteBranch(this.branchName, force);
    }
}
class MergeItem {
    constructor(ref) {
        this.ref = ref;
    }
    get label() { return this.ref.name || ''; }
    get description() { return this.ref.name || ''; }
    async run(repository) {
        await repository.merge(this.ref.name || this.ref.commit);
    }
}
class CreateBranchItem {
    constructor(cc) {
        this.cc = cc;
    }
    get label() { return localize('create branch', '$(plus) Create new branch'); }
    get description() { return ''; }
    get alwaysShow() { return true; }
    async run(repository) {
        await this.cc.branch(repository);
    }
}
class HEADItem {
    constructor(repository) {
        this.repository = repository;
    }
    get label() { return 'HEAD'; }
    get description() { return (this.repository.HEAD && this.repository.HEAD.commit || '').substr(0, 8); }
    get alwaysShow() { return true; }
}
const Commands = [];
function command(commandId, options = {}) {
    return (_target, key, descriptor) => {
        if (!(typeof descriptor.value === 'function')) {
            throw new Error('not supported');
        }
        Commands.push({ commandId, key, method: descriptor.value, options });
    };
}
const ImageMimetypes = [
    'image/png',
    'image/gif',
    'image/jpeg',
    'image/webp',
    'image/tiff',
    'image/bmp'
];
async function categorizeResourceByResolution(resources) {
    const selection = resources.filter(s => s instanceof repository_1.Resource);
    const merge = selection.filter(s => s.resourceGroupType === 0 /* Merge */);
    const isBothAddedOrModified = (s) => s.type === 16 /* BOTH_MODIFIED */ || s.type === 14 /* BOTH_ADDED */;
    const isAnyDeleted = (s) => s.type === 13 /* DELETED_BY_THEM */ || s.type === 12 /* DELETED_BY_US */;
    const possibleUnresolved = merge.filter(isBothAddedOrModified);
    const promises = possibleUnresolved.map(s => util_1.grep(s.resourceUri.fsPath, /^<{7}|^={7}|^>{7}/));
    const unresolvedBothModified = await Promise.all(promises);
    const resolved = possibleUnresolved.filter((_s, i) => !unresolvedBothModified[i]);
    const deletionConflicts = merge.filter(s => isAnyDeleted(s));
    const unresolved = [
        ...merge.filter(s => !isBothAddedOrModified(s) && !isAnyDeleted(s)),
        ...possibleUnresolved.filter((_s, i) => unresolvedBothModified[i])
    ];
    return { merge, resolved, unresolved, deletionConflicts };
}
function createCheckoutItems(repository) {
    const config = vscode_1.workspace.getConfiguration('git');
    const checkoutType = config.get('checkoutType') || 'all';
    const includeTags = checkoutType === 'all' || checkoutType === 'tags';
    const includeRemotes = checkoutType === 'all' || checkoutType === 'remote';
    const heads = repository.refs.filter(ref => ref.type === 0 /* Head */)
        .map(ref => new CheckoutItem(ref));
    const tags = (includeTags ? repository.refs.filter(ref => ref.type === 2 /* Tag */) : [])
        .map(ref => new CheckoutTagItem(ref));
    const remoteHeads = (includeRemotes ? repository.refs.filter(ref => ref.type === 1 /* RemoteHead */) : [])
        .map(ref => new CheckoutRemoteHeadItem(ref));
    return [...heads, ...tags, ...remoteHeads];
}
var PushType;
(function (PushType) {
    PushType[PushType["Push"] = 0] = "Push";
    PushType[PushType["PushTo"] = 1] = "PushTo";
    PushType[PushType["PushTags"] = 2] = "PushTags";
})(PushType || (PushType = {}));
class CommandCenter {
    constructor(git, model, outputChannel, telemetryReporter) {
        this.git = git;
        this.model = model;
        this.outputChannel = outputChannel;
        this.telemetryReporter = telemetryReporter;
        this.disposables = Commands.map(({ commandId, key, method, options }) => {
            const command = this.createCommand(commandId, key, method, options);
            if (options.diff) {
                return vscode_1.commands.registerDiffInformationCommand(commandId, command);
            }
            else {
                return vscode_1.commands.registerCommand(commandId, command);
            }
        });
    }
    async refresh(repository) {
        await repository.status();
    }
    async openResource(resource) {
        const repository = this.model.getRepository(resource.resourceUri);
        if (!repository) {
            return;
        }
        const config = vscode_1.workspace.getConfiguration('git', vscode_1.Uri.file(repository.root));
        const openDiffOnClick = config.get('openDiffOnClick');
        if (openDiffOnClick) {
            await this._openResource(resource, undefined, true, false);
        }
        else {
            await this.openFile(resource);
        }
    }
    async _openResource(resource, preview, preserveFocus, preserveSelection) {
        let stat;
        try {
            stat = await new Promise((c, e) => fs_1.lstat(resource.resourceUri.fsPath, (err, stat) => err ? e(err) : c(stat)));
        }
        catch (err) {
            // noop
        }
        let left;
        let right;
        if (stat && stat.isDirectory()) {
            const repository = this.model.getRepositoryForSubmodule(resource.resourceUri);
            if (repository) {
                right = uri_1.toGitUri(resource.resourceUri, resource.resourceGroupType === 1 /* Index */ ? 'index' : 'wt', { submoduleOf: repository.root });
            }
        }
        else {
            if (resource.type !== 13 /* DELETED_BY_THEM */) {
                left = await this.getLeftResource(resource);
            }
            right = await this.getRightResource(resource);
        }
        const title = this.getTitle(resource);
        if (!right) {
            // TODO
            console.error('oh no');
            return;
        }
        const opts = {
            preserveFocus,
            preview,
            viewColumn: vscode_1.ViewColumn.Active
        };
        const activeTextEditor = vscode_1.window.activeTextEditor;
        // Check if active text editor has same path as other editor. we cannot compare via
        // URI.toString() here because the schemas can be different. Instead we just go by path.
        if (preserveSelection && activeTextEditor && activeTextEditor.document.uri.path === right.path) {
            opts.selection = activeTextEditor.selection;
        }
        if (!left) {
            await vscode_1.commands.executeCommand('vscode.open', right, opts, title);
        }
        else {
            await vscode_1.commands.executeCommand('vscode.diff', left, right, title, opts);
        }
    }
    async getURI(uri, ref) {
        const repository = this.model.getRepository(uri);
        if (!repository) {
            return uri_1.toGitUri(uri, ref);
        }
        try {
            let gitRef = ref;
            if (gitRef === '~') {
                const uriString = uri.toString();
                const [indexStatus] = repository.indexGroup.resourceStates.filter(r => r.resourceUri.toString() === uriString);
                gitRef = indexStatus ? '' : 'HEAD';
            }
            const { size, object } = await repository.getObjectDetails(gitRef, uri.fsPath);
            const { mimetype } = await repository.detectObjectType(object);
            if (mimetype === 'text/plain') {
                return uri_1.toGitUri(uri, ref);
            }
            if (size > 1000000) { // 1 MB
                return vscode_1.Uri.parse(`data:;label:${path.basename(uri.fsPath)};description:${gitRef},`);
            }
            if (ImageMimetypes.indexOf(mimetype) > -1) {
                const contents = await repository.buffer(gitRef, uri.fsPath);
                return vscode_1.Uri.parse(`data:${mimetype};label:${path.basename(uri.fsPath)};description:${gitRef};size:${size};base64,${contents.toString('base64')}`);
            }
            return vscode_1.Uri.parse(`data:;label:${path.basename(uri.fsPath)};description:${gitRef},`);
        }
        catch (err) {
            return uri_1.toGitUri(uri, ref);
        }
    }
    async getLeftResource(resource) {
        switch (resource.type) {
            case 0 /* INDEX_MODIFIED */:
            case 3 /* INDEX_RENAMED */:
                return this.getURI(resource.original, 'HEAD');
            case 5 /* MODIFIED */:
                return this.getURI(resource.resourceUri, '~');
            case 13 /* DELETED_BY_THEM */:
                return this.getURI(resource.resourceUri, '');
        }
        return undefined;
    }
    async getRightResource(resource) {
        switch (resource.type) {
            case 0 /* INDEX_MODIFIED */:
            case 1 /* INDEX_ADDED */:
            case 4 /* INDEX_COPIED */:
            case 3 /* INDEX_RENAMED */:
                return this.getURI(resource.resourceUri, '');
            case 2 /* INDEX_DELETED */:
            case 6 /* DELETED */:
                return this.getURI(resource.resourceUri, 'HEAD');
            case 12 /* DELETED_BY_US */:
                return this.getURI(resource.resourceUri, '~3');
            case 13 /* DELETED_BY_THEM */:
                return this.getURI(resource.resourceUri, '~2');
            case 5 /* MODIFIED */:
            case 7 /* UNTRACKED */:
            case 8 /* IGNORED */:
            case 9 /* INTENT_TO_ADD */:
                const repository = this.model.getRepository(resource.resourceUri);
                if (!repository) {
                    return;
                }
                const uriString = resource.resourceUri.toString();
                const [indexStatus] = repository.indexGroup.resourceStates.filter(r => r.resourceUri.toString() === uriString);
                if (indexStatus && indexStatus.renameResourceUri) {
                    return indexStatus.renameResourceUri;
                }
                return resource.resourceUri;
            case 14 /* BOTH_ADDED */:
            case 16 /* BOTH_MODIFIED */:
                return resource.resourceUri;
        }
        return undefined;
    }
    getTitle(resource) {
        const basename = path.basename(resource.resourceUri.fsPath);
        switch (resource.type) {
            case 0 /* INDEX_MODIFIED */:
            case 3 /* INDEX_RENAMED */:
                return `${basename} (Index)`;
            case 5 /* MODIFIED */:
            case 14 /* BOTH_ADDED */:
            case 16 /* BOTH_MODIFIED */:
                return `${basename} (Working Tree)`;
            case 12 /* DELETED_BY_US */:
                return `${basename} (Theirs)`;
            case 13 /* DELETED_BY_THEM */:
                return `${basename} (Ours)`;
        }
        return '';
    }
    async clone(url) {
        if (!url) {
            url = await vscode_1.window.showInputBox({
                prompt: localize('repourl', "Repository URL"),
                ignoreFocusOut: true
            });
        }
        if (!url) {
            /* __GDPR__
                "clone" : {
                    "outcome" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" }
                }
            */
            this.telemetryReporter.sendTelemetryEvent('clone', { outcome: 'no_URL' });
            return;
        }
        const config = vscode_1.workspace.getConfiguration('git');
        let defaultCloneDirectory = config.get('defaultCloneDirectory') || os.homedir();
        defaultCloneDirectory = defaultCloneDirectory.replace(/^~/, os.homedir());
        const uris = await vscode_1.window.showOpenDialog({
            canSelectFiles: false,
            canSelectFolders: true,
            canSelectMany: false,
            defaultUri: vscode_1.Uri.file(defaultCloneDirectory),
            openLabel: localize('selectFolder', "Select Repository Location")
        });
        if (!uris || uris.length === 0) {
            /* __GDPR__
                "clone" : {
                    "outcome" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" }
                }
            */
            this.telemetryReporter.sendTelemetryEvent('clone', { outcome: 'no_directory' });
            return;
        }
        const uri = uris[0];
        const parentPath = uri.fsPath;
        try {
            const opts = {
                location: vscode_1.ProgressLocation.Notification,
                title: localize('cloning', "Cloning git repository '{0}'...", url),
                cancellable: true
            };
            const repositoryPath = await vscode_1.window.withProgress(opts, (_, token) => this.git.clone(url, parentPath, token));
            const choices = [];
            let message = localize('proposeopen', "Would you like to open the cloned repository?");
            const open = localize('openrepo', "Open Repository");
            choices.push(open);
            const addToWorkspace = localize('add', "Add to Workspace");
            if (vscode_1.workspace.workspaceFolders) {
                message = localize('proposeopen2', "Would you like to open the cloned repository, or add it to the current workspace?");
                choices.push(addToWorkspace);
            }
            const result = await vscode_1.window.showInformationMessage(message, ...choices);
            const openFolder = result === open;
            /* __GDPR__
                "clone" : {
                    "outcome" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" },
                    "openFolder": { "classification": "SystemMetaData", "purpose": "PerformanceAndHealth", "isMeasurement": true }
                }
            */
            this.telemetryReporter.sendTelemetryEvent('clone', { outcome: 'success' }, { openFolder: openFolder ? 1 : 0 });
            const uri = vscode_1.Uri.file(repositoryPath);
            if (openFolder) {
                vscode_1.commands.executeCommand('vscode.openFolder', uri);
            }
            else if (result === addToWorkspace) {
                vscode_1.workspace.updateWorkspaceFolders(vscode_1.workspace.workspaceFolders.length, 0, { uri });
            }
        }
        catch (err) {
            if (/already exists and is not an empty directory/.test(err && err.stderr || '')) {
                /* __GDPR__
                    "clone" : {
                        "outcome" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" }
                    }
                */
                this.telemetryReporter.sendTelemetryEvent('clone', { outcome: 'directory_not_empty' });
            }
            else if (/Cancelled/i.test(err && (err.message || err.stderr || ''))) {
                return;
            }
            else {
                /* __GDPR__
                    "clone" : {
                        "outcome" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" }
                    }
                */
                this.telemetryReporter.sendTelemetryEvent('clone', { outcome: 'error' });
            }
            throw err;
        }
    }
    async init() {
        let repositoryPath = undefined;
        let askToOpen = true;
        if (vscode_1.workspace.workspaceFolders) {
            const placeHolder = localize('init', "Pick workspace folder to initialize git repo in");
            const pick = { label: localize('choose', "Choose Folder...") };
            const items = [
                ...vscode_1.workspace.workspaceFolders.map(folder => ({ label: folder.name, description: folder.uri.fsPath, folder })),
                pick
            ];
            const item = await vscode_1.window.showQuickPick(items, { placeHolder, ignoreFocusOut: true });
            if (!item) {
                return;
            }
            else if (item.folder) {
                repositoryPath = item.folder.uri.fsPath;
                askToOpen = false;
            }
        }
        if (!repositoryPath) {
            const homeUri = vscode_1.Uri.file(os.homedir());
            const defaultUri = vscode_1.workspace.workspaceFolders && vscode_1.workspace.workspaceFolders.length > 0
                ? vscode_1.Uri.file(vscode_1.workspace.workspaceFolders[0].uri.fsPath)
                : homeUri;
            const result = await vscode_1.window.showOpenDialog({
                canSelectFiles: false,
                canSelectFolders: true,
                canSelectMany: false,
                defaultUri,
                openLabel: localize('init repo', "Initialize Repository")
            });
            if (!result || result.length === 0) {
                return;
            }
            const uri = result[0];
            if (homeUri.toString().startsWith(uri.toString())) {
                const yes = localize('create repo', "Initialize Repository");
                const answer = await vscode_1.window.showWarningMessage(localize('are you sure', "This will create a Git repository in '{0}'. Are you sure you want to continue?", uri.fsPath), yes);
                if (answer !== yes) {
                    return;
                }
            }
            repositoryPath = uri.fsPath;
            if (vscode_1.workspace.workspaceFolders && vscode_1.workspace.workspaceFolders.some(w => w.uri.toString() === uri.toString())) {
                askToOpen = false;
            }
        }
        await this.git.init(repositoryPath);
        const choices = [];
        let message = localize('proposeopen init', "Would you like to open the initialized repository?");
        const open = localize('openrepo', "Open Repository");
        choices.push(open);
        if (!askToOpen) {
            return;
        }
        const addToWorkspace = localize('add', "Add to Workspace");
        if (vscode_1.workspace.workspaceFolders) {
            message = localize('proposeopen2 init', "Would you like to open the initialized repository, or add it to the current workspace?");
            choices.push(addToWorkspace);
        }
        const result = await vscode_1.window.showInformationMessage(message, ...choices);
        const uri = vscode_1.Uri.file(repositoryPath);
        if (result === open) {
            vscode_1.commands.executeCommand('vscode.openFolder', uri);
        }
        else if (result === addToWorkspace) {
            vscode_1.workspace.updateWorkspaceFolders(vscode_1.workspace.workspaceFolders.length, 0, { uri });
        }
        else {
            await this.model.openRepository(repositoryPath);
        }
    }
    async openRepository(path) {
        if (!path) {
            const result = await vscode_1.window.showOpenDialog({
                canSelectFiles: false,
                canSelectFolders: true,
                canSelectMany: false,
                defaultUri: vscode_1.Uri.file(os.homedir()),
                openLabel: localize('open repo', "Open Repository")
            });
            if (!result || result.length === 0) {
                return;
            }
            path = result[0].fsPath;
        }
        await this.model.openRepository(path);
    }
    async close(repository) {
        this.model.close(repository);
    }
    async openFile(arg, ...resourceStates) {
        const preserveFocus = arg instanceof repository_1.Resource;
        let uris;
        if (arg instanceof vscode_1.Uri) {
            if (arg.scheme === 'git') {
                uris = [vscode_1.Uri.file(uri_1.fromGitUri(arg).path)];
            }
            else if (arg.scheme === 'file') {
                uris = [arg];
            }
        }
        else {
            let resource = arg;
            if (!(resource instanceof repository_1.Resource)) {
                // can happen when called from a keybinding
                console.log('WHAT');
                resource = this.getSCMResource();
            }
            if (resource) {
                uris = [resource, ...resourceStates]
                    .filter(r => r.type !== 6 /* DELETED */ && r.type !== 2 /* INDEX_DELETED */)
                    .map(r => r.resourceUri);
            }
            else if (vscode_1.window.activeTextEditor) {
                uris = [vscode_1.window.activeTextEditor.document.uri];
            }
        }
        if (!uris) {
            return;
        }
        const activeTextEditor = vscode_1.window.activeTextEditor;
        for (const uri of uris) {
            const opts = {
                preserveFocus,
                preview: false,
                viewColumn: vscode_1.ViewColumn.Active
            };
            const document = await vscode_1.workspace.openTextDocument(uri);
            // Check if active text editor has same path as other editor. we cannot compare via
            // URI.toString() here because the schemas can be different. Instead we just go by path.
            if (activeTextEditor && activeTextEditor.document.uri.path === uri.path) {
                // preserve not only selection but also visible range
                opts.selection = activeTextEditor.selection;
                const previousVisibleRanges = activeTextEditor.visibleRanges;
                const editor = await vscode_1.window.showTextDocument(document, opts);
                editor.revealRange(previousVisibleRanges[0]);
            }
            else {
                await vscode_1.window.showTextDocument(document, opts);
            }
        }
    }
    async openFile2(arg, ...resourceStates) {
        this.openFile(arg, ...resourceStates);
    }
    async openHEADFile(arg) {
        let resource = undefined;
        const preview = !(arg instanceof repository_1.Resource);
        if (arg instanceof repository_1.Resource) {
            resource = arg;
        }
        else if (arg instanceof vscode_1.Uri) {
            resource = this.getSCMResource(arg);
        }
        else {
            resource = this.getSCMResource();
        }
        if (!resource) {
            return;
        }
        const HEAD = await this.getLeftResource(resource);
        if (!HEAD) {
            vscode_1.window.showWarningMessage(localize('HEAD not available', "HEAD version of '{0}' is not available.", path.basename(resource.resourceUri.fsPath)));
            return;
        }
        const opts = {
            preview
        };
        return await vscode_1.commands.executeCommand('vscode.open', HEAD, opts);
    }
    async openChange(arg, ...resourceStates) {
        const preserveFocus = arg instanceof repository_1.Resource;
        const preview = !(arg instanceof repository_1.Resource);
        const preserveSelection = arg instanceof vscode_1.Uri || !arg;
        let resources = undefined;
        if (arg instanceof vscode_1.Uri) {
            const resource = this.getSCMResource(arg);
            if (resource !== undefined) {
                resources = [resource];
            }
        }
        else {
            let resource = undefined;
            if (arg instanceof repository_1.Resource) {
                resource = arg;
            }
            else {
                resource = this.getSCMResource();
            }
            if (resource) {
                resources = [...resourceStates, resource];
            }
        }
        if (!resources) {
            return;
        }
        for (const resource of resources) {
            await this._openResource(resource, preview, preserveFocus, preserveSelection);
        }
    }
    async stage(...resourceStates) {
        this.outputChannel.appendLine(`git.stage ${resourceStates.length}`);
        resourceStates = resourceStates.filter(s => !!s);
        if (resourceStates.length === 0 || (resourceStates[0] && !(resourceStates[0].resourceUri instanceof vscode_1.Uri))) {
            const resource = this.getSCMResource();
            this.outputChannel.appendLine(`git.stage.getSCMResource ${resource ? resource.resourceUri.toString() : null}`);
            if (!resource) {
                return;
            }
            resourceStates = [resource];
        }
        const selection = resourceStates.filter(s => s instanceof repository_1.Resource);
        const { resolved, unresolved, deletionConflicts } = await categorizeResourceByResolution(selection);
        if (unresolved.length > 0) {
            const message = unresolved.length > 1
                ? localize('confirm stage files with merge conflicts', "Are you sure you want to stage {0} files with merge conflicts?", unresolved.length)
                : localize('confirm stage file with merge conflicts', "Are you sure you want to stage {0} with merge conflicts?", path.basename(unresolved[0].resourceUri.fsPath));
            const yes = localize('yes', "Yes");
            const pick = await vscode_1.window.showWarningMessage(message, { modal: true }, yes);
            if (pick !== yes) {
                return;
            }
        }
        try {
            await this.runByRepository(deletionConflicts.map(r => r.resourceUri), async (repository, resources) => {
                for (const resource of resources) {
                    await this._stageDeletionConflict(repository, resource);
                }
            });
        }
        catch (err) {
            if (/Cancelled/.test(err.message)) {
                return;
            }
            throw err;
        }
        const workingTree = selection.filter(s => s.resourceGroupType === 2 /* WorkingTree */);
        const scmResources = [...workingTree, ...resolved, ...unresolved];
        this.outputChannel.appendLine(`git.stage.scmResources ${scmResources.length}`);
        if (!scmResources.length) {
            return;
        }
        const resources = scmResources.map(r => r.resourceUri);
        await this.runByRepository(resources, async (repository, resources) => repository.add(resources));
    }
    async stageAll(repository) {
        const resources = repository.mergeGroup.resourceStates.filter(s => s instanceof repository_1.Resource);
        const { merge, unresolved, deletionConflicts } = await categorizeResourceByResolution(resources);
        try {
            for (const deletionConflict of deletionConflicts) {
                await this._stageDeletionConflict(repository, deletionConflict.resourceUri);
            }
        }
        catch (err) {
            if (/Cancelled/.test(err.message)) {
                return;
            }
            throw err;
        }
        if (unresolved.length > 0) {
            const message = unresolved.length > 1
                ? localize('confirm stage files with merge conflicts', "Are you sure you want to stage {0} files with merge conflicts?", merge.length)
                : localize('confirm stage file with merge conflicts', "Are you sure you want to stage {0} with merge conflicts?", path.basename(merge[0].resourceUri.fsPath));
            const yes = localize('yes', "Yes");
            const pick = await vscode_1.window.showWarningMessage(message, { modal: true }, yes);
            if (pick !== yes) {
                return;
            }
        }
        await repository.add([]);
    }
    async _stageDeletionConflict(repository, uri) {
        const uriString = uri.toString();
        const resource = repository.mergeGroup.resourceStates.filter(r => r.resourceUri.toString() === uriString)[0];
        if (!resource) {
            return;
        }
        if (resource.type === 13 /* DELETED_BY_THEM */) {
            const keepIt = localize('keep ours', "Keep Our Version");
            const deleteIt = localize('delete', "Delete File");
            const result = await vscode_1.window.showInformationMessage(localize('deleted by them', "File '{0}' was deleted by them and modified by us.\n\nWhat would you like to do?", path.basename(uri.fsPath)), { modal: true }, keepIt, deleteIt);
            if (result === keepIt) {
                await repository.add([uri]);
            }
            else if (result === deleteIt) {
                await repository.rm([uri]);
            }
            else {
                throw new Error('Cancelled');
            }
        }
        else if (resource.type === 12 /* DELETED_BY_US */) {
            const keepIt = localize('keep theirs', "Keep Their Version");
            const deleteIt = localize('delete', "Delete File");
            const result = await vscode_1.window.showInformationMessage(localize('deleted by us', "File '{0}' was deleted by us and modified by them.\n\nWhat would you like to do?", path.basename(uri.fsPath)), { modal: true }, keepIt, deleteIt);
            if (result === keepIt) {
                await repository.add([uri]);
            }
            else if (result === deleteIt) {
                await repository.rm([uri]);
            }
            else {
                throw new Error('Cancelled');
            }
        }
    }
    async stageChange(uri, changes, index) {
        const textEditor = vscode_1.window.visibleTextEditors.filter(e => e.document.uri.toString() === uri.toString())[0];
        if (!textEditor) {
            return;
        }
        await this._stageChanges(textEditor, [changes[index]]);
    }
    async stageSelectedChanges(changes) {
        const textEditor = vscode_1.window.activeTextEditor;
        if (!textEditor) {
            return;
        }
        const modifiedDocument = textEditor.document;
        const selectedLines = staging_1.toLineRanges(textEditor.selections, modifiedDocument);
        const selectedChanges = changes
            .map(diff => selectedLines.reduce((result, range) => result || staging_1.intersectDiffWithRange(modifiedDocument, diff, range), null))
            .filter(d => !!d);
        if (!selectedChanges.length) {
            return;
        }
        await this._stageChanges(textEditor, selectedChanges);
    }
    async _stageChanges(textEditor, changes) {
        const modifiedDocument = textEditor.document;
        const modifiedUri = modifiedDocument.uri;
        if (modifiedUri.scheme !== 'file') {
            return;
        }
        const originalUri = uri_1.toGitUri(modifiedUri, '~');
        const originalDocument = await vscode_1.workspace.openTextDocument(originalUri);
        const result = staging_1.applyLineChanges(originalDocument, modifiedDocument, changes);
        await this.runByRepository(modifiedUri, async (repository, resource) => await repository.stage(resource, result));
    }
    async revertChange(uri, changes, index) {
        const textEditor = vscode_1.window.visibleTextEditors.filter(e => e.document.uri.toString() === uri.toString())[0];
        if (!textEditor) {
            return;
        }
        await this._revertChanges(textEditor, [...changes.slice(0, index), ...changes.slice(index + 1)]);
    }
    async revertSelectedRanges(changes) {
        const textEditor = vscode_1.window.activeTextEditor;
        if (!textEditor) {
            return;
        }
        const modifiedDocument = textEditor.document;
        const selections = textEditor.selections;
        const selectedChanges = changes.filter(change => {
            const modifiedRange = staging_1.getModifiedRange(modifiedDocument, change);
            return selections.every(selection => !selection.intersection(modifiedRange));
        });
        if (selectedChanges.length === changes.length) {
            return;
        }
        await this._revertChanges(textEditor, selectedChanges);
    }
    async _revertChanges(textEditor, changes) {
        const modifiedDocument = textEditor.document;
        const modifiedUri = modifiedDocument.uri;
        if (modifiedUri.scheme !== 'file') {
            return;
        }
        const originalUri = uri_1.toGitUri(modifiedUri, '~');
        const originalDocument = await vscode_1.workspace.openTextDocument(originalUri);
        const selectionsBeforeRevert = textEditor.selections;
        const visibleRangesBeforeRevert = textEditor.visibleRanges;
        const result = staging_1.applyLineChanges(originalDocument, modifiedDocument, changes);
        const edit = new vscode_1.WorkspaceEdit();
        edit.replace(modifiedUri, new vscode_1.Range(new vscode_1.Position(0, 0), modifiedDocument.lineAt(modifiedDocument.lineCount - 1).range.end), result);
        vscode_1.workspace.applyEdit(edit);
        await modifiedDocument.save();
        textEditor.selections = selectionsBeforeRevert;
        textEditor.revealRange(visibleRangesBeforeRevert[0]);
    }
    async unstage(...resourceStates) {
        resourceStates = resourceStates.filter(s => !!s);
        if (resourceStates.length === 0 || (resourceStates[0] && !(resourceStates[0].resourceUri instanceof vscode_1.Uri))) {
            const resource = this.getSCMResource();
            if (!resource) {
                return;
            }
            resourceStates = [resource];
        }
        const scmResources = resourceStates
            .filter(s => s instanceof repository_1.Resource && s.resourceGroupType === 1 /* Index */);
        if (!scmResources.length) {
            return;
        }
        const resources = scmResources.map(r => r.resourceUri);
        await this.runByRepository(resources, async (repository, resources) => repository.revert(resources));
    }
    async unstageAll(repository) {
        await repository.revert([]);
    }
    async unstageSelectedRanges(diffs) {
        const textEditor = vscode_1.window.activeTextEditor;
        if (!textEditor) {
            return;
        }
        const modifiedDocument = textEditor.document;
        const modifiedUri = modifiedDocument.uri;
        if (modifiedUri.scheme !== 'git') {
            return;
        }
        const { ref } = uri_1.fromGitUri(modifiedUri);
        if (ref !== '') {
            return;
        }
        const originalUri = uri_1.toGitUri(modifiedUri, 'HEAD');
        const originalDocument = await vscode_1.workspace.openTextDocument(originalUri);
        const selectedLines = staging_1.toLineRanges(textEditor.selections, modifiedDocument);
        const selectedDiffs = diffs
            .map(diff => selectedLines.reduce((result, range) => result || staging_1.intersectDiffWithRange(modifiedDocument, diff, range), null))
            .filter(d => !!d);
        if (!selectedDiffs.length) {
            return;
        }
        const invertedDiffs = selectedDiffs.map(staging_1.invertLineChange);
        const result = staging_1.applyLineChanges(modifiedDocument, originalDocument, invertedDiffs);
        await this.runByRepository(modifiedUri, async (repository, resource) => await repository.stage(resource, result));
    }
    async clean(...resourceStates) {
        resourceStates = resourceStates.filter(s => !!s);
        if (resourceStates.length === 0 || (resourceStates[0] && !(resourceStates[0].resourceUri instanceof vscode_1.Uri))) {
            const resource = this.getSCMResource();
            if (!resource) {
                return;
            }
            resourceStates = [resource];
        }
        const scmResources = resourceStates
            .filter(s => s instanceof repository_1.Resource && s.resourceGroupType === 2 /* WorkingTree */);
        if (!scmResources.length) {
            return;
        }
        const untrackedCount = scmResources.reduce((s, r) => s + (r.type === 7 /* UNTRACKED */ ? 1 : 0), 0);
        let message;
        let yes = localize('discard', "Discard Changes");
        if (scmResources.length === 1) {
            if (untrackedCount > 0) {
                message = localize('confirm delete', "Are you sure you want to DELETE {0}?", path.basename(scmResources[0].resourceUri.fsPath));
                yes = localize('delete file', "Delete file");
            }
            else {
                if (scmResources[0].type === 6 /* DELETED */) {
                    yes = localize('restore file', "Restore file");
                    message = localize('confirm restore', "Are you sure you want to restore {0}?", path.basename(scmResources[0].resourceUri.fsPath));
                }
                else {
                    message = localize('confirm discard', "Are you sure you want to discard changes in {0}?", path.basename(scmResources[0].resourceUri.fsPath));
                }
            }
        }
        else {
            if (scmResources.every(resource => resource.type === 6 /* DELETED */)) {
                yes = localize('restore files', "Restore files");
                message = localize('confirm restore multiple', "Are you sure you want to restore {0} files?", scmResources.length);
            }
            else {
                message = localize('confirm discard multiple', "Are you sure you want to discard changes in {0} files?", scmResources.length);
            }
            if (untrackedCount > 0) {
                message = `${message}\n\n${localize('warn untracked', "This will DELETE {0} untracked files!", untrackedCount)}`;
            }
        }
        const pick = await vscode_1.window.showWarningMessage(message, { modal: true }, yes);
        if (pick !== yes) {
            return;
        }
        const resources = scmResources.map(r => r.resourceUri);
        await this.runByRepository(resources, async (repository, resources) => repository.clean(resources));
    }
    async cleanAll(repository) {
        let resources = repository.workingTreeGroup.resourceStates;
        if (resources.length === 0) {
            return;
        }
        const trackedResources = resources.filter(r => r.type !== 7 /* UNTRACKED */ && r.type !== 8 /* IGNORED */);
        const untrackedResources = resources.filter(r => r.type === 7 /* UNTRACKED */ || r.type === 8 /* IGNORED */);
        if (untrackedResources.length === 0) {
            const message = resources.length === 1
                ? localize('confirm discard all single', "Are you sure you want to discard changes in {0}?", path.basename(resources[0].resourceUri.fsPath))
                : localize('confirm discard all', "Are you sure you want to discard ALL changes in {0} files?\nThis is IRREVERSIBLE!\nYour current working set will be FOREVER LOST.", resources.length);
            const yes = resources.length === 1
                ? localize('discardAll multiple', "Discard 1 File")
                : localize('discardAll', "Discard All {0} Files", resources.length);
            const pick = await vscode_1.window.showWarningMessage(message, { modal: true }, yes);
            if (pick !== yes) {
                return;
            }
            await repository.clean(resources.map(r => r.resourceUri));
            return;
        }
        else if (resources.length === 1) {
            const message = localize('confirm delete', "Are you sure you want to DELETE {0}?", path.basename(resources[0].resourceUri.fsPath));
            const yes = localize('delete file', "Delete file");
            const pick = await vscode_1.window.showWarningMessage(message, { modal: true }, yes);
            if (pick !== yes) {
                return;
            }
            await repository.clean(resources.map(r => r.resourceUri));
        }
        else if (trackedResources.length === 0) {
            const message = localize('confirm delete multiple', "Are you sure you want to DELETE {0} files?", resources.length);
            const yes = localize('delete files', "Delete Files");
            const pick = await vscode_1.window.showWarningMessage(message, { modal: true }, yes);
            if (pick !== yes) {
                return;
            }
            await repository.clean(resources.map(r => r.resourceUri));
        }
        else { // resources.length > 1 && untrackedResources.length > 0 && trackedResources.length > 0
            const untrackedMessage = untrackedResources.length === 1
                ? localize('there are untracked files single', "The following untracked file will be DELETED FROM DISK if discarded: {0}.", path.basename(untrackedResources[0].resourceUri.fsPath))
                : localize('there are untracked files', "There are {0} untracked files which will be DELETED FROM DISK if discarded.", untrackedResources.length);
            const message = localize('confirm discard all 2', "{0}\n\nThis is IRREVERSIBLE, your current working set will be FOREVER LOST.", untrackedMessage, resources.length);
            const yesTracked = trackedResources.length === 1
                ? localize('yes discard tracked', "Discard 1 Tracked File", trackedResources.length)
                : localize('yes discard tracked multiple', "Discard {0} Tracked Files", trackedResources.length);
            const yesAll = localize('discardAll', "Discard All {0} Files", resources.length);
            const pick = await vscode_1.window.showWarningMessage(message, { modal: true }, yesTracked, yesAll);
            if (pick === yesTracked) {
                resources = trackedResources;
            }
            else if (pick !== yesAll) {
                return;
            }
            await repository.clean(resources.map(r => r.resourceUri));
        }
    }
    async smartCommit(repository, getCommitMessage, opts) {
        const config = vscode_1.workspace.getConfiguration('git', vscode_1.Uri.file(repository.root));
        const promptToSaveFilesBeforeCommit = config.get('promptToSaveFilesBeforeCommit') === true;
        if (promptToSaveFilesBeforeCommit) {
            const unsavedTextDocuments = vscode_1.workspace.textDocuments
                .filter(d => !d.isUntitled && d.isDirty && util_1.isDescendant(repository.root, d.uri.fsPath));
            if (unsavedTextDocuments.length > 0) {
                const message = unsavedTextDocuments.length === 1
                    ? localize('unsaved files single', "The following file is unsaved: {0}.\n\nWould you like to save it before committing?", path.basename(unsavedTextDocuments[0].uri.fsPath))
                    : localize('unsaved files', "There are {0} unsaved files.\n\nWould you like to save them before committing?", unsavedTextDocuments.length);
                const saveAndCommit = localize('save and commit', "Save All & Commit");
                const commit = localize('commit', "Commit Anyway");
                const pick = await vscode_1.window.showWarningMessage(message, { modal: true }, saveAndCommit, commit);
                if (pick === saveAndCommit) {
                    await Promise.all(unsavedTextDocuments.map(d => d.save()));
                    await repository.status();
                }
                else if (pick !== commit) {
                    return false; // do not commit on cancel
                }
            }
        }
        const enableSmartCommit = config.get('enableSmartCommit') === true;
        const enableCommitSigning = config.get('enableCommitSigning') === true;
        const noStagedChanges = repository.indexGroup.resourceStates.length === 0;
        const noUnstagedChanges = repository.workingTreeGroup.resourceStates.length === 0;
        // no changes, and the user has not configured to commit all in this case
        if (!noUnstagedChanges && noStagedChanges && !enableSmartCommit) {
            // prompt the user if we want to commit all or not
            const message = localize('no staged changes', "There are no staged changes to commit.\n\nWould you like to automatically stage all your changes and commit them directly?");
            const yes = localize('yes', "Yes");
            const always = localize('always', "Always");
            const pick = await vscode_1.window.showWarningMessage(message, { modal: true }, yes, always);
            if (pick === always) {
                config.update('enableSmartCommit', true, true);
            }
            else if (pick !== yes) {
                return false; // do not commit on cancel
            }
        }
        if (!opts) {
            opts = { all: noStagedChanges };
        }
        else if (!opts.all && noStagedChanges) {
            opts = Object.assign({}, opts, { all: true });
        }
        // enable signing of commits if configurated
        opts.signCommit = enableCommitSigning;
        if (config.get('alwaysSignOff')) {
            opts.signoff = true;
        }
        if ((
        // no changes
        (noStagedChanges && noUnstagedChanges)
            // or no staged changes and not `all`
            || (!opts.all && noStagedChanges))
            && !opts.empty) {
            vscode_1.window.showInformationMessage(localize('no changes', "There are no changes to commit."));
            return false;
        }
        const message = await getCommitMessage();
        if (!message) {
            return false;
        }
        await repository.commit(message, opts);
        const postCommitCommand = config.get('postCommitCommand');
        switch (postCommitCommand) {
            case 'push':
                await this._push(repository, { pushType: PushType.Push, silent: true });
                break;
            case 'sync':
                await this.sync(repository);
                break;
        }
        return true;
    }
    async commitWithAnyInput(repository, opts) {
        const message = repository.inputBox.value;
        const getCommitMessage = async () => {
            if (message) {
                return message;
            }
            let value = undefined;
            if (opts && opts.amend && repository.HEAD && repository.HEAD.commit) {
                value = (await repository.getCommit(repository.HEAD.commit)).message;
            }
            return await vscode_1.window.showInputBox({
                value,
                placeHolder: localize('commit message', "Commit message"),
                prompt: localize('provide commit message', "Please provide a commit message"),
                ignoreFocusOut: true
            });
        };
        const didCommit = await this.smartCommit(repository, getCommitMessage, opts);
        if (message && didCommit) {
            repository.inputBox.value = await repository.getCommitTemplate();
        }
    }
    async commit(repository) {
        await this.commitWithAnyInput(repository);
    }
    async commitWithInput(repository) {
        if (!repository.inputBox.value) {
            return;
        }
        const didCommit = await this.smartCommit(repository, async () => repository.inputBox.value);
        if (didCommit) {
            repository.inputBox.value = await repository.getCommitTemplate();
        }
    }
    async commitStaged(repository) {
        await this.commitWithAnyInput(repository, { all: false });
    }
    async commitStagedSigned(repository) {
        await this.commitWithAnyInput(repository, { all: false, signoff: true });
    }
    async commitStagedAmend(repository) {
        await this.commitWithAnyInput(repository, { all: false, amend: true });
    }
    async commitAll(repository) {
        await this.commitWithAnyInput(repository, { all: true });
    }
    async commitAllSigned(repository) {
        await this.commitWithAnyInput(repository, { all: true, signoff: true });
    }
    async commitAllAmend(repository) {
        await this.commitWithAnyInput(repository, { all: true, amend: true });
    }
    async commitEmpty(repository) {
        const root = vscode_1.Uri.file(repository.root);
        const config = vscode_1.workspace.getConfiguration('git', root);
        const shouldPrompt = config.get('confirmEmptyCommits') === true;
        if (shouldPrompt) {
            const message = localize('confirm emtpy commit', "Are you sure you want to create an empty commit?");
            const yes = localize('yes', "Yes");
            const neverAgain = localize('yes never again', "Yes, Don't Show Again");
            const pick = await vscode_1.window.showWarningMessage(message, { modal: true }, yes, neverAgain);
            if (pick === neverAgain) {
                await config.update('confirmEmptyCommits', false, true);
            }
            else if (pick !== yes) {
                return;
            }
        }
        await this.commitWithAnyInput(repository, { empty: true });
    }
    async restoreCommitTemplate(repository) {
        repository.inputBox.value = await repository.getCommitTemplate();
    }
    async undoCommit(repository) {
        const HEAD = repository.HEAD;
        if (!HEAD || !HEAD.commit) {
            vscode_1.window.showWarningMessage(localize('no more', "Can't undo because HEAD doesn't point to any commit."));
            return;
        }
        const commit = await repository.getCommit('HEAD');
        if (commit.parents.length > 0) {
            await repository.reset('HEAD~');
        }
        else {
            await repository.deleteRef('HEAD');
            await this.unstageAll(repository);
        }
        repository.inputBox.value = commit.message;
    }
    async checkout(repository, treeish) {
        if (typeof treeish === 'string') {
            await repository.checkout(treeish);
            return true;
        }
        const createBranch = new CreateBranchItem(this);
        const picks = [createBranch, ...createCheckoutItems(repository)];
        const placeHolder = localize('select a ref to checkout', 'Select a ref to checkout');
        const quickpick = vscode_1.window.createQuickPick();
        quickpick.items = picks;
        quickpick.placeholder = placeHolder;
        quickpick.show();
        const choice = await new Promise(c => quickpick.onDidAccept(() => c(quickpick.activeItems[0])));
        quickpick.hide();
        if (!choice) {
            return false;
        }
        if (choice === createBranch) {
            await this._branch(repository, quickpick.value);
        }
        else {
            await choice.run(repository);
        }
        return true;
    }
    async branch(repository) {
        await this._branch(repository);
    }
    async _branch(repository, defaultName) {
        const config = vscode_1.workspace.getConfiguration('git');
        const branchWhitespaceChar = config.get('branchWhitespaceChar');
        const branchValidationRegex = config.get('branchValidationRegex');
        const sanitize = (name) => name ?
            name.trim().replace(/^\.|\/\.|\.\.|~|\^|:|\/$|\.lock$|\.lock\/|\\|\*|\s|^\s*$|\.$|\[|\]$/g, branchWhitespaceChar)
            : name;
        const rawBranchName = defaultName || await vscode_1.window.showInputBox({
            placeHolder: localize('branch name', "Branch name"),
            prompt: localize('provide branch name', "Please provide a branch name"),
            ignoreFocusOut: true,
            validateInput: (name) => {
                const validateName = new RegExp(branchValidationRegex);
                if (validateName.test(sanitize(name))) {
                    return null;
                }
                return localize('branch name format invalid', "Branch name needs to match regex: {0}", branchValidationRegex);
            }
        });
        const branchName = sanitize(rawBranchName || '');
        if (!branchName) {
            return;
        }
        const picks = [new HEADItem(repository), ...createCheckoutItems(repository)];
        const placeHolder = localize('select a ref to create a new branch from', 'Select a ref to create the \'{0}\' branch from', branchName);
        const target = await vscode_1.window.showQuickPick(picks, { placeHolder });
        if (!target) {
            return;
        }
        await repository.branch(branchName, true, target.label);
    }
    async deleteBranch(repository, name, force) {
        let run;
        if (typeof name === 'string') {
            run = force => repository.deleteBranch(name, force);
        }
        else {
            const currentHead = repository.HEAD && repository.HEAD.name;
            const heads = repository.refs.filter(ref => ref.type === 0 /* Head */ && ref.name !== currentHead)
                .map(ref => new BranchDeleteItem(ref));
            const placeHolder = localize('select branch to delete', 'Select a branch to delete');
            const choice = await vscode_1.window.showQuickPick(heads, { placeHolder });
            if (!choice || !choice.branchName) {
                return;
            }
            name = choice.branchName;
            run = force => choice.run(repository, force);
        }
        try {
            await run(force);
        }
        catch (err) {
            if (err.gitErrorCode !== "BranchNotFullyMerged" /* BranchNotFullyMerged */) {
                throw err;
            }
            const message = localize('confirm force delete branch', "The branch '{0}' is not fully merged. Delete anyway?", name);
            const yes = localize('delete branch', "Delete Branch");
            const pick = await vscode_1.window.showWarningMessage(message, { modal: true }, yes);
            if (pick === yes) {
                await run(true);
            }
        }
    }
    async renameBranch(repository) {
        const placeHolder = localize('provide branch name', "Please provide a branch name");
        const name = await vscode_1.window.showInputBox({ placeHolder });
        if (!name || name.trim().length === 0) {
            return;
        }
        try {
            await repository.renameBranch(name);
        }
        catch (err) {
            switch (err.gitErrorCode) {
                case "InvalidBranchName" /* InvalidBranchName */:
                    vscode_1.window.showErrorMessage(localize('invalid branch name', 'Invalid branch name'));
                    return;
                case "BranchAlreadyExists" /* BranchAlreadyExists */:
                    vscode_1.window.showErrorMessage(localize('branch already exists', "A branch named '{0}' already exists", name));
                    return;
                default:
                    throw err;
            }
        }
    }
    async merge(repository) {
        const config = vscode_1.workspace.getConfiguration('git');
        const checkoutType = config.get('checkoutType') || 'all';
        const includeRemotes = checkoutType === 'all' || checkoutType === 'remote';
        const heads = repository.refs.filter(ref => ref.type === 0 /* Head */)
            .filter(ref => ref.name || ref.commit)
            .map(ref => new MergeItem(ref));
        const remoteHeads = (includeRemotes ? repository.refs.filter(ref => ref.type === 1 /* RemoteHead */) : [])
            .filter(ref => ref.name || ref.commit)
            .map(ref => new MergeItem(ref));
        const picks = [...heads, ...remoteHeads];
        const placeHolder = localize('select a branch to merge from', 'Select a branch to merge from');
        const choice = await vscode_1.window.showQuickPick(picks, { placeHolder });
        if (!choice) {
            return;
        }
        await choice.run(repository);
    }
    async createTag(repository) {
        const inputTagName = await vscode_1.window.showInputBox({
            placeHolder: localize('tag name', "Tag name"),
            prompt: localize('provide tag name', "Please provide a tag name"),
            ignoreFocusOut: true
        });
        if (!inputTagName) {
            return;
        }
        const inputMessage = await vscode_1.window.showInputBox({
            placeHolder: localize('tag message', "Message"),
            prompt: localize('provide tag message', "Please provide a message to annotate the tag"),
            ignoreFocusOut: true
        });
        const name = inputTagName.replace(/^\.|\/\.|\.\.|~|\^|:|\/$|\.lock$|\.lock\/|\\|\*|\s|^\s*$|\.$/g, '-');
        const message = inputMessage || name;
        await repository.tag(name, message);
    }
    async fetch(repository) {
        if (repository.remotes.length === 0) {
            vscode_1.window.showWarningMessage(localize('no remotes to fetch', "This repository has no remotes configured to fetch from."));
            return;
        }
        await repository.fetchDefault();
    }
    async fetchPrune(repository) {
        if (repository.remotes.length === 0) {
            vscode_1.window.showWarningMessage(localize('no remotes to fetch', "This repository has no remotes configured to fetch from."));
            return;
        }
        await repository.fetchPrune();
    }
    async fetchAll(repository) {
        if (repository.remotes.length === 0) {
            vscode_1.window.showWarningMessage(localize('no remotes to fetch', "This repository has no remotes configured to fetch from."));
            return;
        }
        await repository.fetchAll();
    }
    async pullFrom(repository) {
        const remotes = repository.remotes;
        if (remotes.length === 0) {
            vscode_1.window.showWarningMessage(localize('no remotes to pull', "Your repository has no remotes configured to pull from."));
            return;
        }
        const remotePicks = remotes.filter(r => r.fetchUrl !== undefined).map(r => ({ label: r.name, description: r.fetchUrl }));
        const placeHolder = localize('pick remote pull repo', "Pick a remote to pull the branch from");
        const remotePick = await vscode_1.window.showQuickPick(remotePicks, { placeHolder });
        if (!remotePick) {
            return;
        }
        const remoteRefs = repository.refs;
        const remoteRefsFiltered = remoteRefs.filter(r => (r.remote === remotePick.label));
        const branchPicks = remoteRefsFiltered.map(r => ({ label: r.name }));
        const branchPlaceHolder = localize('pick branch pull', "Pick a branch to pull from");
        const branchPick = await vscode_1.window.showQuickPick(branchPicks, { placeHolder: branchPlaceHolder });
        if (!branchPick) {
            return;
        }
        const remoteCharCnt = remotePick.label.length;
        await repository.pullFrom(false, remotePick.label, branchPick.label.slice(remoteCharCnt + 1));
    }
    async pull(repository) {
        const remotes = repository.remotes;
        if (remotes.length === 0) {
            vscode_1.window.showWarningMessage(localize('no remotes to pull', "Your repository has no remotes configured to pull from."));
            return;
        }
        await repository.pull(repository.HEAD);
    }
    async pullRebase(repository) {
        const remotes = repository.remotes;
        if (remotes.length === 0) {
            vscode_1.window.showWarningMessage(localize('no remotes to pull', "Your repository has no remotes configured to pull from."));
            return;
        }
        await repository.pullWithRebase(repository.HEAD);
    }
    async _push(repository, pushOptions) {
        const remotes = repository.remotes;
        if (remotes.length === 0) {
            if (!pushOptions.silent) {
                vscode_1.window.showWarningMessage(localize('no remotes to push', "Your repository has no remotes configured to push to."));
            }
            return;
        }
        const config = vscode_1.workspace.getConfiguration('git', vscode_1.Uri.file(repository.root));
        let forcePushMode = undefined;
        if (pushOptions.forcePush) {
            if (!config.get('allowForcePush')) {
                await vscode_1.window.showErrorMessage(localize('force push not allowed', "Force push is not allowed, please enable it with the 'git.allowForcePush' setting."));
                return;
            }
            forcePushMode = config.get('useForcePushWithLease') === true ? git_1.ForcePushMode.ForceWithLease : git_1.ForcePushMode.Force;
            if (config.get('confirmForcePush')) {
                const message = localize('confirm force push', "You are about to force push your changes, this can be destructive and could inadvertedly overwrite changes made by others.\n\nAre you sure to continue?");
                const yes = localize('ok', "OK");
                const neverAgain = localize('never ask again', "OK, Don't Ask Again");
                const pick = await vscode_1.window.showWarningMessage(message, { modal: true }, yes, neverAgain);
                if (pick === neverAgain) {
                    config.update('confirmForcePush', false, true);
                }
                else if (pick !== yes) {
                    return;
                }
            }
        }
        if (pushOptions.pushType === PushType.PushTags) {
            await repository.pushTags(undefined, forcePushMode);
            vscode_1.window.showInformationMessage(localize('push with tags success', "Successfully pushed with tags."));
            return;
        }
        if (!repository.HEAD || !repository.HEAD.name) {
            if (!pushOptions.silent) {
                vscode_1.window.showWarningMessage(localize('nobranch', "Please check out a branch to push to a remote."));
            }
            return;
        }
        if (pushOptions.pushType === PushType.Push) {
            try {
                await repository.push(repository.HEAD, forcePushMode);
            }
            catch (err) {
                if (err.gitErrorCode !== "NoUpstreamBranch" /* NoUpstreamBranch */) {
                    throw err;
                }
                if (pushOptions.silent) {
                    return;
                }
                const branchName = repository.HEAD.name;
                const message = localize('confirm publish branch', "The branch '{0}' has no upstream branch. Would you like to publish this branch?", branchName);
                const yes = localize('ok', "OK");
                const pick = await vscode_1.window.showWarningMessage(message, { modal: true }, yes);
                if (pick === yes) {
                    await this.publish(repository);
                }
            }
        }
        else {
            const branchName = repository.HEAD.name;
            const picks = remotes.filter(r => r.pushUrl !== undefined).map(r => ({ label: r.name, description: r.pushUrl }));
            const placeHolder = localize('pick remote', "Pick a remote to publish the branch '{0}' to:", branchName);
            const pick = await vscode_1.window.showQuickPick(picks, { placeHolder });
            if (!pick) {
                return;
            }
            await repository.pushTo(pick.label, branchName, undefined, forcePushMode);
        }
    }
    async push(repository) {
        await this._push(repository, { pushType: PushType.Push });
    }
    async pushForce(repository) {
        await this._push(repository, { pushType: PushType.Push, forcePush: true });
    }
    async pushWithTags(repository) {
        await this._push(repository, { pushType: PushType.PushTags });
    }
    async pushWithTagsForce(repository) {
        await this._push(repository, { pushType: PushType.PushTags, forcePush: true });
    }
    async pushTo(repository) {
        await this._push(repository, { pushType: PushType.PushTo });
    }
    async pushToForce(repository) {
        await this._push(repository, { pushType: PushType.PushTo, forcePush: true });
    }
    async addRemote(repository) {
        const remotes = repository.remotes;
        const sanitize = (name) => {
            name = name.trim();
            return name && name.replace(/^\.|\/\.|\.\.|~|\^|:|\/$|\.lock$|\.lock\/|\\|\*|\s|^\s*$|\.$|\[|\]$/g, '-');
        };
        const resultName = await vscode_1.window.showInputBox({
            placeHolder: localize('remote name', "Remote name"),
            prompt: localize('provide remote name', "Please provide a remote name"),
            ignoreFocusOut: true,
            validateInput: (name) => {
                if (sanitize(name)) {
                    return null;
                }
                return localize('remote name format invalid', "Remote name format invalid");
            }
        });
        const name = sanitize(resultName || '');
        if (!name) {
            return;
        }
        if (remotes.find(r => r.name === name)) {
            vscode_1.window.showErrorMessage(localize('remote already exists', "Remote '{0}' already exists.", name));
            return;
        }
        const url = await vscode_1.window.showInputBox({
            placeHolder: localize('remote url', "Remote URL"),
            prompt: localize('provide remote URL', "Enter URL for remote \"{0}\"", name),
            ignoreFocusOut: true
        });
        if (!url) {
            return;
        }
        await repository.addRemote(name, url);
    }
    async removeRemote(repository) {
        const remotes = repository.remotes;
        if (remotes.length === 0) {
            vscode_1.window.showErrorMessage(localize('no remotes added', "Your repository has no remotes."));
            return;
        }
        const picks = remotes.map(r => r.name);
        const placeHolder = localize('remove remote', "Pick a remote to remove");
        const remoteName = await vscode_1.window.showQuickPick(picks, { placeHolder });
        if (!remoteName) {
            return;
        }
        await repository.removeRemote(remoteName);
    }
    async _sync(repository, rebase) {
        const HEAD = repository.HEAD;
        if (!HEAD || !HEAD.upstream) {
            return;
        }
        const remoteName = HEAD.remote || HEAD.upstream.remote;
        const remote = repository.remotes.find(r => r.name === remoteName);
        const isReadonly = remote && remote.isReadOnly;
        const config = vscode_1.workspace.getConfiguration('git');
        const shouldPrompt = !isReadonly && config.get('confirmSync') === true;
        if (shouldPrompt) {
            const message = localize('sync is unpredictable', "This action will push and pull commits to and from '{0}/{1}'.", HEAD.upstream.remote, HEAD.upstream.name);
            const yes = localize('ok', "OK");
            const neverAgain = localize('never again', "OK, Don't Show Again");
            const pick = await vscode_1.window.showWarningMessage(message, { modal: true }, yes, neverAgain);
            if (pick === neverAgain) {
                await config.update('confirmSync', false, true);
            }
            else if (pick !== yes) {
                return;
            }
        }
        if (rebase) {
            await repository.syncRebase(HEAD);
        }
        else {
            await repository.sync(HEAD);
        }
    }
    sync(repository) {
        return this._sync(repository, false);
    }
    async syncAll() {
        await Promise.all(this.model.repositories.map(async (repository) => {
            const HEAD = repository.HEAD;
            if (!HEAD || !HEAD.upstream) {
                return;
            }
            await repository.sync(HEAD);
        }));
    }
    syncRebase(repository) {
        return this._sync(repository, true);
    }
    async publish(repository) {
        const remotes = repository.remotes;
        if (remotes.length === 0) {
            vscode_1.window.showWarningMessage(localize('no remotes to publish', "Your repository has no remotes configured to publish to."));
            return;
        }
        const branchName = repository.HEAD && repository.HEAD.name || '';
        const selectRemote = async () => {
            const picks = repository.remotes.map(r => r.name);
            const placeHolder = localize('pick remote', "Pick a remote to publish the branch '{0}' to:", branchName);
            return await vscode_1.window.showQuickPick(picks, { placeHolder });
        };
        const choice = remotes.length === 1 ? remotes[0].name : await selectRemote();
        if (!choice) {
            return;
        }
        await repository.pushTo(choice, branchName, true);
    }
    async ignore(...resourceStates) {
        resourceStates = resourceStates.filter(s => !!s);
        if (resourceStates.length === 0 || (resourceStates[0] && !(resourceStates[0].resourceUri instanceof vscode_1.Uri))) {
            const resource = this.getSCMResource();
            if (!resource) {
                return;
            }
            resourceStates = [resource];
        }
        const resources = resourceStates
            .filter(s => s instanceof repository_1.Resource)
            .map(r => r.resourceUri);
        if (!resources.length) {
            return;
        }
        await this.runByRepository(resources, async (repository, resources) => repository.ignore(resources));
    }
    async _stash(repository, includeUntracked = false) {
        const noUnstagedChanges = repository.workingTreeGroup.resourceStates.length === 0;
        const noStagedChanges = repository.indexGroup.resourceStates.length === 0;
        if (noUnstagedChanges && noStagedChanges) {
            vscode_1.window.showInformationMessage(localize('no changes stash', "There are no changes to stash."));
            return;
        }
        const message = await this.getStashMessage();
        if (typeof message === 'undefined') {
            return;
        }
        await repository.createStash(message, includeUntracked);
    }
    async getStashMessage() {
        return await vscode_1.window.showInputBox({
            prompt: localize('provide stash message', "Optionally provide a stash message"),
            placeHolder: localize('stash message', "Stash message")
        });
    }
    stash(repository) {
        return this._stash(repository);
    }
    stashIncludeUntracked(repository) {
        return this._stash(repository, true);
    }
    async stashPop(repository) {
        const placeHolder = localize('pick stash to pop', "Pick a stash to pop");
        const stash = await this.pickStash(repository, placeHolder);
        if (!stash) {
            return;
        }
        await repository.popStash(stash.index);
    }
    async stashPopLatest(repository) {
        const stashes = await repository.getStashes();
        if (stashes.length === 0) {
            vscode_1.window.showInformationMessage(localize('no stashes', "There are no stashes in the repository."));
            return;
        }
        await repository.popStash();
    }
    async stashApply(repository) {
        const placeHolder = localize('pick stash to apply', "Pick a stash to apply");
        const stash = await this.pickStash(repository, placeHolder);
        if (!stash) {
            return;
        }
        await repository.applyStash(stash.index);
    }
    async stashApplyLatest(repository) {
        const stashes = await repository.getStashes();
        if (stashes.length === 0) {
            vscode_1.window.showInformationMessage(localize('no stashes', "There are no stashes in the repository."));
            return;
        }
        await repository.applyStash();
    }
    async pickStash(repository, placeHolder) {
        const stashes = await repository.getStashes();
        if (stashes.length === 0) {
            vscode_1.window.showInformationMessage(localize('no stashes', "There are no stashes in the repository."));
            return;
        }
        const picks = stashes.map(stash => ({ label: `#${stash.index}:  ${stash.description}`, description: '', details: '', stash }));
        const result = await vscode_1.window.showQuickPick(picks, { placeHolder });
        return result && result.stash;
    }
    createCommand(id, key, method, options) {
        const result = (...args) => {
            let result;
            if (!options.repository) {
                result = Promise.resolve(method.apply(this, args));
            }
            else {
                // try to guess the repository based on the first argument
                const repository = this.model.getRepository(args[0]);
                let repositoryPromise;
                if (repository) {
                    repositoryPromise = Promise.resolve(repository);
                }
                else if (this.model.repositories.length === 1) {
                    repositoryPromise = Promise.resolve(this.model.repositories[0]);
                }
                else {
                    repositoryPromise = this.model.pickRepository();
                }
                result = repositoryPromise.then(repository => {
                    if (!repository) {
                        return Promise.resolve();
                    }
                    return Promise.resolve(method.apply(this, [repository, ...args]));
                });
            }
            /* __GDPR__
                "git.command" : {
                    "command" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" }
                }
            */
            this.telemetryReporter.sendTelemetryEvent('git.command', { command: id });
            return result.catch(async (err) => {
                const options = {
                    modal: true
                };
                let message;
                let type = 'error';
                const choices = new Map();
                const openOutputChannelChoice = localize('open git log', "Open Git Log");
                const outputChannel = this.outputChannel;
                choices.set(openOutputChannelChoice, () => outputChannel.show());
                switch (err.gitErrorCode) {
                    case "DirtyWorkTree" /* DirtyWorkTree */:
                        message = localize('clean repo', "Please clean your repository working tree before checkout.");
                        break;
                    case "PushRejected" /* PushRejected */:
                        message = localize('cant push', "Can't push refs to remote. Try running 'Pull' first to integrate your changes.");
                        break;
                    case "Conflict" /* Conflict */:
                        message = localize('merge conflicts', "There are merge conflicts. Resolve them before committing.");
                        type = 'warning';
                        options.modal = false;
                        break;
                    case "StashConflict" /* StashConflict */:
                        message = localize('stash merge conflicts', "There were merge conflicts while applying the stash.");
                        type = 'warning';
                        options.modal = false;
                        break;
                    case "NoUserNameConfigured" /* NoUserNameConfigured */:
                    case "NoUserEmailConfigured" /* NoUserEmailConfigured */:
                        message = localize('missing user info', "Make sure you configure your 'user.name' and 'user.email' in git.");
                        choices.set(localize('learn more', "Learn More"), () => vscode_1.commands.executeCommand('vscode.open', vscode_1.Uri.parse('https://git-scm.com/book/en/v2/Getting-Started-First-Time-Git-Setup')));
                        break;
                    default:
                        const hint = (err.stderr || err.message || String(err))
                            .replace(/^error: /mi, '')
                            .replace(/^> husky.*$/mi, '')
                            .split(/[\r\n]/)
                            .filter((line) => !!line)[0];
                        message = hint
                            ? localize('git error details', "Git: {0}", hint)
                            : localize('git error', "Git error");
                        break;
                }
                if (!message) {
                    console.error(err);
                    return;
                }
                const allChoices = Array.from(choices.keys());
                const result = type === 'error'
                    ? await vscode_1.window.showErrorMessage(message, options, ...allChoices)
                    : await vscode_1.window.showWarningMessage(message, options, ...allChoices);
                if (result) {
                    const resultFn = choices.get(result);
                    if (resultFn) {
                        resultFn();
                    }
                }
            });
        };
        // patch this object, so people can call methods directly
        this[key] = result;
        return result;
    }
    getSCMResource(uri) {
        uri = uri ? uri : (vscode_1.window.activeTextEditor && vscode_1.window.activeTextEditor.document.uri);
        this.outputChannel.appendLine(`git.getSCMResource.uri ${uri && uri.toString()}`);
        for (const r of this.model.repositories.map(r => r.root)) {
            this.outputChannel.appendLine(`repo root ${r}`);
        }
        if (!uri) {
            return undefined;
        }
        if (uri.scheme === 'git') {
            const { path } = uri_1.fromGitUri(uri);
            uri = vscode_1.Uri.file(path);
        }
        if (uri.scheme === 'file') {
            const uriString = uri.toString();
            const repository = this.model.getRepository(uri);
            if (!repository) {
                return undefined;
            }
            return repository.workingTreeGroup.resourceStates.filter(r => r.resourceUri.toString() === uriString)[0]
                || repository.indexGroup.resourceStates.filter(r => r.resourceUri.toString() === uriString)[0];
        }
        return undefined;
    }
    async runByRepository(arg, fn) {
        const resources = arg instanceof vscode_1.Uri ? [arg] : arg;
        const isSingleResource = arg instanceof vscode_1.Uri;
        const groups = resources.reduce((result, resource) => {
            let repository = this.model.getRepository(resource);
            if (!repository) {
                console.warn('Could not find git repository for ', resource);
                return result;
            }
            // Could it be a submodule?
            if (util_1.pathEquals(resource.fsPath, repository.root)) {
                repository = this.model.getRepositoryForSubmodule(resource) || repository;
            }
            const tuple = result.filter(p => p.repository === repository)[0];
            if (tuple) {
                tuple.resources.push(resource);
            }
            else {
                result.push({ repository, resources: [resource] });
            }
            return result;
        }, []);
        const promises = groups
            .map(({ repository, resources }) => fn(repository, isSingleResource ? resources[0] : resources));
        return Promise.all(promises);
    }
    dispose() {
        this.disposables.forEach(d => d.dispose());
    }
}
__decorate([
    command('git.refresh', { repository: true })
], CommandCenter.prototype, "refresh", null);
__decorate([
    command('git.openResource')
], CommandCenter.prototype, "openResource", null);
__decorate([
    command('git.clone')
], CommandCenter.prototype, "clone", null);
__decorate([
    command('git.init')
], CommandCenter.prototype, "init", null);
__decorate([
    command('git.openRepository', { repository: false })
], CommandCenter.prototype, "openRepository", null);
__decorate([
    command('git.close', { repository: true })
], CommandCenter.prototype, "close", null);
__decorate([
    command('git.openFile')
], CommandCenter.prototype, "openFile", null);
__decorate([
    command('git.openFile2')
], CommandCenter.prototype, "openFile2", null);
__decorate([
    command('git.openHEADFile')
], CommandCenter.prototype, "openHEADFile", null);
__decorate([
    command('git.openChange')
], CommandCenter.prototype, "openChange", null);
__decorate([
    command('git.stage')
], CommandCenter.prototype, "stage", null);
__decorate([
    command('git.stageAll', { repository: true })
], CommandCenter.prototype, "stageAll", null);
__decorate([
    command('git.stageChange')
], CommandCenter.prototype, "stageChange", null);
__decorate([
    command('git.stageSelectedRanges', { diff: true })
], CommandCenter.prototype, "stageSelectedChanges", null);
__decorate([
    command('git.revertChange')
], CommandCenter.prototype, "revertChange", null);
__decorate([
    command('git.revertSelectedRanges', { diff: true })
], CommandCenter.prototype, "revertSelectedRanges", null);
__decorate([
    command('git.unstage')
], CommandCenter.prototype, "unstage", null);
__decorate([
    command('git.unstageAll', { repository: true })
], CommandCenter.prototype, "unstageAll", null);
__decorate([
    command('git.unstageSelectedRanges', { diff: true })
], CommandCenter.prototype, "unstageSelectedRanges", null);
__decorate([
    command('git.clean')
], CommandCenter.prototype, "clean", null);
__decorate([
    command('git.cleanAll', { repository: true })
], CommandCenter.prototype, "cleanAll", null);
__decorate([
    command('git.commit', { repository: true })
], CommandCenter.prototype, "commit", null);
__decorate([
    command('git.commitWithInput', { repository: true })
], CommandCenter.prototype, "commitWithInput", null);
__decorate([
    command('git.commitStaged', { repository: true })
], CommandCenter.prototype, "commitStaged", null);
__decorate([
    command('git.commitStagedSigned', { repository: true })
], CommandCenter.prototype, "commitStagedSigned", null);
__decorate([
    command('git.commitStagedAmend', { repository: true })
], CommandCenter.prototype, "commitStagedAmend", null);
__decorate([
    command('git.commitAll', { repository: true })
], CommandCenter.prototype, "commitAll", null);
__decorate([
    command('git.commitAllSigned', { repository: true })
], CommandCenter.prototype, "commitAllSigned", null);
__decorate([
    command('git.commitAllAmend', { repository: true })
], CommandCenter.prototype, "commitAllAmend", null);
__decorate([
    command('git.commitEmpty', { repository: true })
], CommandCenter.prototype, "commitEmpty", null);
__decorate([
    command('git.restoreCommitTemplate', { repository: true })
], CommandCenter.prototype, "restoreCommitTemplate", null);
__decorate([
    command('git.undoCommit', { repository: true })
], CommandCenter.prototype, "undoCommit", null);
__decorate([
    command('git.checkout', { repository: true })
], CommandCenter.prototype, "checkout", null);
__decorate([
    command('git.branch', { repository: true })
], CommandCenter.prototype, "branch", null);
__decorate([
    command('git.deleteBranch', { repository: true })
], CommandCenter.prototype, "deleteBranch", null);
__decorate([
    command('git.renameBranch', { repository: true })
], CommandCenter.prototype, "renameBranch", null);
__decorate([
    command('git.merge', { repository: true })
], CommandCenter.prototype, "merge", null);
__decorate([
    command('git.createTag', { repository: true })
], CommandCenter.prototype, "createTag", null);
__decorate([
    command('git.fetch', { repository: true })
], CommandCenter.prototype, "fetch", null);
__decorate([
    command('git.fetchPrune', { repository: true })
], CommandCenter.prototype, "fetchPrune", null);
__decorate([
    command('git.fetchAll', { repository: true })
], CommandCenter.prototype, "fetchAll", null);
__decorate([
    command('git.pullFrom', { repository: true })
], CommandCenter.prototype, "pullFrom", null);
__decorate([
    command('git.pull', { repository: true })
], CommandCenter.prototype, "pull", null);
__decorate([
    command('git.pullRebase', { repository: true })
], CommandCenter.prototype, "pullRebase", null);
__decorate([
    command('git.push', { repository: true })
], CommandCenter.prototype, "push", null);
__decorate([
    command('git.pushForce', { repository: true })
], CommandCenter.prototype, "pushForce", null);
__decorate([
    command('git.pushWithTags', { repository: true })
], CommandCenter.prototype, "pushWithTags", null);
__decorate([
    command('git.pushWithTagsForce', { repository: true })
], CommandCenter.prototype, "pushWithTagsForce", null);
__decorate([
    command('git.pushTo', { repository: true })
], CommandCenter.prototype, "pushTo", null);
__decorate([
    command('git.pushToForce', { repository: true })
], CommandCenter.prototype, "pushToForce", null);
__decorate([
    command('git.addRemote', { repository: true })
], CommandCenter.prototype, "addRemote", null);
__decorate([
    command('git.removeRemote', { repository: true })
], CommandCenter.prototype, "removeRemote", null);
__decorate([
    command('git.sync', { repository: true })
], CommandCenter.prototype, "sync", null);
__decorate([
    command('git._syncAll')
], CommandCenter.prototype, "syncAll", null);
__decorate([
    command('git.syncRebase', { repository: true })
], CommandCenter.prototype, "syncRebase", null);
__decorate([
    command('git.publish', { repository: true })
], CommandCenter.prototype, "publish", null);
__decorate([
    command('git.ignore')
], CommandCenter.prototype, "ignore", null);
__decorate([
    command('git.stash', { repository: true })
], CommandCenter.prototype, "stash", null);
__decorate([
    command('git.stashIncludeUntracked', { repository: true })
], CommandCenter.prototype, "stashIncludeUntracked", null);
__decorate([
    command('git.stashPop', { repository: true })
], CommandCenter.prototype, "stashPop", null);
__decorate([
    command('git.stashPopLatest', { repository: true })
], CommandCenter.prototype, "stashPopLatest", null);
__decorate([
    command('git.stashApply', { repository: true })
], CommandCenter.prototype, "stashApply", null);
__decorate([
    command('git.stashApplyLatest', { repository: true })
], CommandCenter.prototype, "stashApplyLatest", null);
exports.CommandCenter = CommandCenter;
//# sourceMappingURL=commands.js.map