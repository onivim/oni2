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
exports.GitTimelineProvider = exports.GitTimelineItem = void 0;
const nls = require("vscode-nls");
const vscode_1 = require("vscode");
const repository_1 = require("./repository");
const decorators_1 = require("./decorators");
const emoji_1 = require("./emoji");
const localize = nls.loadMessageBundle();
class GitTimelineItem extends vscode_1.TimelineItem {
    constructor(ref, previousRef, message, timestamp, id, contextValue) {
        const index = message.indexOf('\n');
        const label = index !== -1 ? `${message.substring(0, index)} \u2026` : message;
        super(label, timestamp);
        this.ref = ref;
        this.previousRef = previousRef;
        this.message = message;
        this.id = id;
        this.contextValue = contextValue;
    }
    static is(item) {
        return item instanceof GitTimelineItem;
    }
    get shortRef() {
        return this.shortenRef(this.ref);
    }
    get shortPreviousRef() {
        return this.shortenRef(this.previousRef);
    }
    shortenRef(ref) {
        if (ref === '' || ref === '~' || ref === 'HEAD') {
            return ref;
        }
        return ref.endsWith('^') ? `${ref.substr(0, 8)}^` : ref.substr(0, 8);
    }
}
exports.GitTimelineItem = GitTimelineItem;
class GitTimelineProvider {
    constructor(model) {
        this.model = model;
        this._onDidChange = new vscode_1.EventEmitter();
        this.id = 'git-history';
        this.label = localize('git.timeline.source', 'Git History');
        this.disposable = vscode_1.Disposable.from(model.onDidOpenRepository(this.onRepositoriesChanged, this), vscode_1.workspace.onDidChangeConfiguration(this.onConfigurationChanged, this));
        if (model.repositories.length) {
            this.ensureProviderRegistration();
        }
    }
    get onDidChange() {
        return this._onDidChange.event;
    }
    dispose() {
        var _a;
        (_a = this.providerDisposable) === null || _a === void 0 ? void 0 : _a.dispose();
        this.disposable.dispose();
    }
    async provideTimeline(uri, options, _token) {
        // console.log(`GitTimelineProvider.provideTimeline: uri=${uri} state=${this._model.state}`);
        var _a, _b, _c, _d, _e;
        const repo = this.model.getRepository(uri);
        if (!repo) {
            (_a = this.repoDisposable) === null || _a === void 0 ? void 0 : _a.dispose();
            this.repoStatusDate = undefined;
            this.repo = undefined;
            return { items: [] };
        }
        if (((_b = this.repo) === null || _b === void 0 ? void 0 : _b.root) !== repo.root) {
            (_c = this.repoDisposable) === null || _c === void 0 ? void 0 : _c.dispose();
            this.repo = repo;
            this.repoStatusDate = new Date();
            this.repoDisposable = vscode_1.Disposable.from(repo.onDidChangeRepository(uri => this.onRepositoryChanged(repo, uri)), repo.onDidRunGitStatus(() => this.onRepositoryStatusChanged(repo)));
        }
        const config = vscode_1.workspace.getConfiguration('git.timeline');
        // TODO@eamodio: Ensure that the uri is a file -- if not we could get the history of the repo?
        let limit;
        if (options.limit !== undefined && typeof options.limit !== 'number') {
            try {
                const result = await this.model.git.exec(repo.root, ['rev-list', '--count', `${options.limit.id}..`, '--', uri.fsPath]);
                if (!result.exitCode) {
                    // Ask for 2 more (1 for the limit commit and 1 for the next commit) than so we can determine if there are more commits
                    limit = Number(result.stdout) + 2;
                }
            }
            catch (_f) {
                limit = undefined;
            }
        }
        else {
            // If we are not getting everything, ask for 1 more than so we can determine if there are more commits
            limit = options.limit === undefined ? undefined : options.limit + 1;
        }
        await emoji_1.ensureEmojis();
        const commits = await repo.logFile(uri, {
            maxEntries: limit,
            hash: options.cursor,
            // sortByAuthorDate: true
        });
        const paging = commits.length ? {
            cursor: limit === undefined ? undefined : (commits.length >= limit ? (_d = commits[commits.length - 1]) === null || _d === void 0 ? void 0 : _d.hash : undefined)
        } : undefined;
        // If we asked for an extra commit, strip it off
        if (limit !== undefined && commits.length >= limit) {
            commits.splice(commits.length - 1, 1);
        }
        const dateFormatter = new Intl.DateTimeFormat(vscode_1.env.language, { year: 'numeric', month: 'long', day: 'numeric', hour: 'numeric', minute: 'numeric' });
        const dateType = config.get('date');
        const showAuthor = config.get('showAuthor');
        const items = commits.map((c, i) => {
            var _a, _b, _c;
            const date = dateType === 'authored' ? c.authorDate : c.commitDate;
            const message = emoji_1.emojify(c.message);
            const item = new GitTimelineItem(c.hash, (_b = (_a = commits[i + 1]) === null || _a === void 0 ? void 0 : _a.hash) !== null && _b !== void 0 ? _b : `${c.hash}^`, message, (_c = date === null || date === void 0 ? void 0 : date.getTime()) !== null && _c !== void 0 ? _c : 0, c.hash, 'git:file:commit');
            item.iconPath = new vscode_1.ThemeIcon('git-commit');
            if (showAuthor) {
                item.description = c.authorName;
            }
            item.detail = `${c.authorName} (${c.authorEmail}) — ${c.hash.substr(0, 8)}\n${dateFormatter.format(date)}\n\n${message}`;
            item.command = {
                title: 'Open Comparison',
                command: 'git.timeline.openDiff',
                arguments: [item, uri, this.id]
            };
            return item;
        });
        if (options.cursor === undefined) {
            const you = localize('git.timeline.you', 'You');
            const index = repo.indexGroup.resourceStates.find(r => r.resourceUri.fsPath === uri.fsPath);
            if (index) {
                const date = (_e = this.repoStatusDate) !== null && _e !== void 0 ? _e : new Date();
                const item = new GitTimelineItem('~', 'HEAD', localize('git.timeline.stagedChanges', 'Staged Changes'), date.getTime(), 'index', 'git:file:index');
                // TODO@eamodio: Replace with a better icon -- reflecting its status maybe?
                item.iconPath = new vscode_1.ThemeIcon('git-commit');
                item.description = '';
                item.detail = localize('git.timeline.detail', '{0}  — {1}\n{2}\n\n{3}', you, localize('git.index', 'Index'), dateFormatter.format(date), repository_1.Resource.getStatusText(index.type));
                item.command = {
                    title: 'Open Comparison',
                    command: 'git.timeline.openDiff',
                    arguments: [item, uri, this.id]
                };
                items.splice(0, 0, item);
            }
            const working = repo.workingTreeGroup.resourceStates.find(r => r.resourceUri.fsPath === uri.fsPath);
            if (working) {
                const date = new Date();
                const item = new GitTimelineItem('', index ? '~' : 'HEAD', localize('git.timeline.uncommitedChanges', 'Uncommitted Changes'), date.getTime(), 'working', 'git:file:working');
                // TODO@eamodio: Replace with a better icon -- reflecting its status maybe?
                item.iconPath = new vscode_1.ThemeIcon('git-commit');
                item.description = '';
                item.detail = localize('git.timeline.detail', '{0}  — {1}\n{2}\n\n{3}', you, localize('git.workingTree', 'Working Tree'), dateFormatter.format(date), repository_1.Resource.getStatusText(working.type));
                item.command = {
                    title: 'Open Comparison',
                    command: 'git.timeline.openDiff',
                    arguments: [item, uri, this.id]
                };
                items.splice(0, 0, item);
            }
        }
        return {
            items: items,
            paging: paging
        };
    }
    ensureProviderRegistration() {
        if (this.providerDisposable === undefined) {
            this.providerDisposable = vscode_1.workspace.registerTimelineProvider(['file', 'git', 'vscode-remote', 'gitlens-git'], this);
        }
    }
    onConfigurationChanged(e) {
        if (e.affectsConfiguration('git.timeline.date') || e.affectsConfiguration('git.timeline.showAuthor')) {
            this.fireChanged();
        }
    }
    onRepositoriesChanged(_repo) {
        // console.log(`GitTimelineProvider.onRepositoriesChanged`);
        this.ensureProviderRegistration();
        // TODO@eamodio: Being naive for now and just always refreshing each time there is a new repository
        this.fireChanged();
    }
    onRepositoryChanged(_repo, _uri) {
        // console.log(`GitTimelineProvider.onRepositoryChanged: uri=${uri.toString(true)}`);
        this.fireChanged();
    }
    onRepositoryStatusChanged(_repo) {
        // console.log(`GitTimelineProvider.onRepositoryStatusChanged`);
        // This is less than ideal, but for now just save the last time a status was run and use that as the timestamp for staged items
        this.repoStatusDate = new Date();
        this.fireChanged();
    }
    fireChanged() {
        this._onDidChange.fire(undefined);
    }
}
__decorate([
    decorators_1.debounce(500)
], GitTimelineProvider.prototype, "fireChanged", null);
exports.GitTimelineProvider = GitTimelineProvider;
//# sourceMappingURL=timelineProvider.js.map