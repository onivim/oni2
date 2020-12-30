"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.DynamicMarkdownPreview = exports.StaticMarkdownPreview = exports.PreviewDocumentVersion = void 0;
const path = require("path");
const vscode = require("vscode");
const nls = require("vscode-nls");
const openDocumentLink_1 = require("../commands/openDocumentLink");
const dispose_1 = require("../util/dispose");
const file_1 = require("../util/file");
const resources_1 = require("../util/resources");
const topmostLineMonitor_1 = require("../util/topmostLineMonitor");
const localize = nls.loadMessageBundle();
class PreviewDocumentVersion {
    constructor(document) {
        this.resource = document.uri;
        this.version = document.version;
    }
    equals(other) {
        return this.resource.fsPath === other.resource.fsPath
            && this.version === other.version;
    }
}
exports.PreviewDocumentVersion = PreviewDocumentVersion;
class StartingScrollLine {
    constructor(line) {
        this.line = line;
        this.type = 'line';
    }
}
class StartingScrollFragment {
    constructor(fragment) {
        this.fragment = fragment;
        this.type = 'fragment';
    }
}
class MarkdownPreview extends dispose_1.Disposable {
    constructor(webview, resource, startingScroll, delegate, engine, _contentProvider, _previewConfigurations, _logger, _contributionProvider) {
        super();
        this.delegate = delegate;
        this.engine = engine;
        this._contentProvider = _contentProvider;
        this._previewConfigurations = _previewConfigurations;
        this._logger = _logger;
        this._contributionProvider = _contributionProvider;
        this.delay = 300;
        this.firstUpdate = true;
        this.isScrolling = false;
        this._disposed = false;
        this.imageInfo = [];
        this._webviewPanel = webview;
        this._resource = resource;
        switch (startingScroll === null || startingScroll === void 0 ? void 0 : startingScroll.type) {
            case 'line':
                if (!isNaN(startingScroll.line)) {
                    this.line = startingScroll.line;
                }
                break;
            case 'fragment':
                this.scrollToFragment = startingScroll.fragment;
                break;
        }
        this._register(_contributionProvider.onContributionsChanged(() => {
            setImmediate(() => this.refresh());
        }));
        this._register(vscode.workspace.onDidChangeTextDocument(event => {
            if (this.isPreviewOf(event.document.uri)) {
                this.refresh();
            }
        }));
        this._register(this._webviewPanel.webview.onDidReceiveMessage((e) => {
            if (e.source !== this._resource.toString()) {
                return;
            }
            switch (e.type) {
                case 'cacheImageSizes':
                    this.imageInfo = e.body;
                    break;
                case 'revealLine':
                    this.onDidScrollPreview(e.body.line);
                    break;
                case 'didClick':
                    this.onDidClickPreview(e.body.line);
                    break;
                case 'openLink':
                    this.onDidClickPreviewLink(e.body.href);
                    break;
                case 'showPreviewSecuritySelector':
                    vscode.commands.executeCommand('markdown.showPreviewSecuritySelector', e.source);
                    break;
                case 'previewStyleLoadError':
                    vscode.window.showWarningMessage(localize('onPreviewStyleLoadError', "Could not load 'markdown.styles': {0}", e.body.unloadedStyles.join(', ')));
                    break;
            }
        }));
        this.updatePreview();
    }
    dispose() {
        super.dispose();
        this._disposed = true;
        clearTimeout(this.throttleTimer);
    }
    get resource() {
        return this._resource;
    }
    get state() {
        return {
            resource: this._resource.toString(),
            line: this.line,
            imageInfo: this.imageInfo,
            fragment: this.scrollToFragment,
            ...this.delegate.getAdditionalState(),
        };
    }
    refresh() {
        // Schedule update if none is pending
        if (!this.throttleTimer) {
            if (this.firstUpdate) {
                this.updatePreview(true);
            }
            else {
                this.throttleTimer = setTimeout(() => this.updatePreview(true), this.delay);
            }
        }
        this.firstUpdate = false;
    }
    get iconPath() {
        const root = vscode.Uri.joinPath(this._contributionProvider.extensionUri, 'media');
        return {
            light: vscode.Uri.joinPath(root, 'preview-light.svg'),
            dark: vscode.Uri.joinPath(root, 'preview-dark.svg'),
        };
    }
    isPreviewOf(resource) {
        return this._resource.fsPath === resource.fsPath;
    }
    postMessage(msg) {
        if (!this._disposed) {
            this._webviewPanel.webview.postMessage(msg);
        }
    }
    scrollTo(topLine) {
        if (this._disposed) {
            return;
        }
        if (this.isScrolling) {
            this.isScrolling = false;
            return;
        }
        this._logger.log('updateForView', { markdownFile: this._resource });
        this.line = topLine;
        this.postMessage({
            type: 'updateView',
            line: topLine,
            source: this._resource.toString()
        });
    }
    async updatePreview(forceUpdate) {
        var _a, _b;
        clearTimeout(this.throttleTimer);
        this.throttleTimer = undefined;
        if (this._disposed) {
            return;
        }
        let document;
        try {
            document = await vscode.workspace.openTextDocument(this._resource);
        }
        catch (_c) {
            await this.showFileNotFoundError();
            return;
        }
        if (this._disposed) {
            return;
        }
        const pendingVersion = new PreviewDocumentVersion(document);
        if (!forceUpdate && ((_a = this.currentVersion) === null || _a === void 0 ? void 0 : _a.equals(pendingVersion))) {
            if (this.line) {
                this.scrollTo(this.line);
            }
            return;
        }
        this.currentVersion = pendingVersion;
        const content = await this._contentProvider.provideTextDocumentContent(document, this, this._previewConfigurations, this.line, this.state);
        // Another call to `doUpdate` may have happened.
        // Make sure we are still updating for the correct document
        if ((_b = this.currentVersion) === null || _b === void 0 ? void 0 : _b.equals(pendingVersion)) {
            this.setContent(content);
        }
    }
    onDidScrollPreview(line) {
        this.line = line;
        const config = this._previewConfigurations.loadAndCacheConfiguration(this._resource);
        if (!config.scrollEditorWithPreview) {
            return;
        }
        for (const editor of vscode.window.visibleTextEditors) {
            if (!this.isPreviewOf(editor.document.uri)) {
                continue;
            }
            this.isScrolling = true;
            const sourceLine = Math.floor(line);
            const fraction = line - sourceLine;
            const text = editor.document.lineAt(sourceLine).text;
            const start = Math.floor(fraction * text.length);
            editor.revealRange(new vscode.Range(sourceLine, start, sourceLine + 1, 0), vscode.TextEditorRevealType.AtTop);
        }
    }
    async onDidClickPreview(line) {
        // fix #82457, find currently opened but unfocused source tab
        await vscode.commands.executeCommand('markdown.showSource');
        for (const visibleEditor of vscode.window.visibleTextEditors) {
            if (this.isPreviewOf(visibleEditor.document.uri)) {
                const editor = await vscode.window.showTextDocument(visibleEditor.document, visibleEditor.viewColumn);
                const position = new vscode.Position(line, 0);
                editor.selection = new vscode.Selection(position, position);
                return;
            }
        }
        vscode.workspace.openTextDocument(this._resource)
            .then(vscode.window.showTextDocument)
            .then(undefined, () => {
            vscode.window.showErrorMessage(localize('preview.clickOpenFailed', 'Could not open {0}', this._resource.toString()));
        });
    }
    async showFileNotFoundError() {
        this._webviewPanel.webview.html = this._contentProvider.provideFileNotFoundContent(this._resource);
    }
    setContent(html) {
        if (this._disposed) {
            return;
        }
        if (this.delegate.getTitle) {
            this._webviewPanel.title = this.delegate.getTitle(this._resource);
        }
        this._webviewPanel.iconPath = this.iconPath;
        this._webviewPanel.webview.options = this.getWebviewOptions();
        this._webviewPanel.webview.html = html;
    }
    getWebviewOptions() {
        return {
            enableScripts: true,
            localResourceRoots: this.getLocalResourceRoots()
        };
    }
    getLocalResourceRoots() {
        var _a;
        const baseRoots = Array.from(this._contributionProvider.contributions.previewResourceRoots);
        const folder = vscode.workspace.getWorkspaceFolder(this._resource);
        if (folder) {
            const workspaceRoots = (_a = vscode.workspace.workspaceFolders) === null || _a === void 0 ? void 0 : _a.map(folder => folder.uri);
            if (workspaceRoots) {
                baseRoots.push(...workspaceRoots);
            }
        }
        else if (!this._resource.scheme || this._resource.scheme === 'file') {
            baseRoots.push(vscode.Uri.file(path.dirname(this._resource.fsPath)));
        }
        return baseRoots.map(root => resources_1.normalizeResource(this._resource, root));
    }
    async onDidClickPreviewLink(href) {
        let [hrefPath, fragment] = decodeURIComponent(href).split('#');
        // We perviously already resolve absolute paths.
        // Now make sure we handle relative file paths
        if (hrefPath[0] !== '/') {
            // Fix #93691, use this.resource.fsPath instead of this.resource.path
            hrefPath = path.join(path.dirname(this.resource.fsPath), hrefPath);
        }
        const config = vscode.workspace.getConfiguration('markdown', this.resource);
        const openLinks = config.get('preview.openMarkdownLinks', 'inPreview');
        if (openLinks === 'inPreview') {
            const markdownLink = await openDocumentLink_1.resolveLinkToMarkdownFile(hrefPath);
            if (markdownLink) {
                this.delegate.openPreviewLinkToMarkdownFile(markdownLink, fragment);
                return;
            }
        }
        openDocumentLink_1.OpenDocumentLinkCommand.execute(this.engine, { parts: { path: hrefPath }, fragment, fromResource: this.resource.toJSON() });
    }
    //#region WebviewResourceProvider
    asWebviewUri(resource) {
        return this._webviewPanel.webview.asWebviewUri(resources_1.normalizeResource(this._resource, resource));
    }
    get cspSource() {
        return this._webviewPanel.webview.cspSource;
    }
}
class StaticMarkdownPreview extends dispose_1.Disposable {
    constructor(_webviewPanel, resource, contentProvider, _previewConfigurations, logger, contributionProvider, engine) {
        super();
        this._webviewPanel = _webviewPanel;
        this._previewConfigurations = _previewConfigurations;
        this._onDispose = this._register(new vscode.EventEmitter());
        this.onDispose = this._onDispose.event;
        this._onDidChangeViewState = this._register(new vscode.EventEmitter());
        this.onDidChangeViewState = this._onDidChangeViewState.event;
        this.preview = this._register(new MarkdownPreview(this._webviewPanel, resource, undefined, {
            getAdditionalState: () => { return {}; },
            openPreviewLinkToMarkdownFile: () => { }
        }, engine, contentProvider, _previewConfigurations, logger, contributionProvider));
        this._register(this._webviewPanel.onDidDispose(() => {
            this.dispose();
        }));
        this._register(this._webviewPanel.onDidChangeViewState(e => {
            this._onDidChangeViewState.fire(e);
        }));
    }
    static revive(resource, webview, contentProvider, previewConfigurations, logger, contributionProvider, engine) {
        return new StaticMarkdownPreview(webview, resource, contentProvider, previewConfigurations, logger, contributionProvider, engine);
    }
    dispose() {
        this._onDispose.fire();
        super.dispose();
    }
    matchesResource(_otherResource, _otherPosition, _otherLocked) {
        return false;
    }
    refresh() {
        this.preview.refresh();
    }
    updateConfiguration() {
        if (this._previewConfigurations.hasConfigurationChanged(this.preview.resource)) {
            this.refresh();
        }
    }
    get resource() {
        return this.preview.resource;
    }
    get resourceColumn() {
        return this._webviewPanel.viewColumn || vscode.ViewColumn.One;
    }
}
exports.StaticMarkdownPreview = StaticMarkdownPreview;
/**
 * A
 */
class DynamicMarkdownPreview extends dispose_1.Disposable {
    constructor(webview, input, _contentProvider, _previewConfigurations, _logger, _topmostLineMonitor, _contributionProvider, _engine) {
        super();
        this._contentProvider = _contentProvider;
        this._previewConfigurations = _previewConfigurations;
        this._logger = _logger;
        this._topmostLineMonitor = _topmostLineMonitor;
        this._contributionProvider = _contributionProvider;
        this._engine = _engine;
        this._onDisposeEmitter = this._register(new vscode.EventEmitter());
        this.onDispose = this._onDisposeEmitter.event;
        this._onDidChangeViewStateEmitter = this._register(new vscode.EventEmitter());
        this.onDidChangeViewState = this._onDidChangeViewStateEmitter.event;
        this._webviewPanel = webview;
        this._resourceColumn = input.resourceColumn;
        this._locked = input.locked;
        this._preview = this.createPreview(input.resource, typeof input.line === 'number' ? new StartingScrollLine(input.line) : undefined);
        this._register(webview.onDidDispose(() => { this.dispose(); }));
        this._register(this._webviewPanel.onDidChangeViewState(e => {
            this._onDidChangeViewStateEmitter.fire(e);
        }));
        this._register(this._topmostLineMonitor.onDidChanged(event => {
            if (this._preview.isPreviewOf(event.resource)) {
                this._preview.scrollTo(event.line);
            }
        }));
        this._register(vscode.window.onDidChangeTextEditorSelection(event => {
            if (this._preview.isPreviewOf(event.textEditor.document.uri)) {
                this._preview.postMessage({
                    type: 'onDidChangeTextEditorSelection',
                    line: event.selections[0].active.line,
                    source: this._preview.resource.toString()
                });
            }
        }));
        this._register(vscode.window.onDidChangeActiveTextEditor(editor => {
            // Only allow previewing normal text editors which have a viewColumn: See #101514
            if (typeof (editor === null || editor === void 0 ? void 0 : editor.viewColumn) === 'undefined') {
                return;
            }
            if (file_1.isMarkdownFile(editor.document) && !this._locked && !this._preview.isPreviewOf(editor.document.uri)) {
                const line = topmostLineMonitor_1.getVisibleLine(editor);
                this.update(editor.document.uri, line ? new StartingScrollLine(line) : undefined);
            }
        }));
    }
    static revive(input, webview, contentProvider, previewConfigurations, logger, topmostLineMonitor, contributionProvider, engine) {
        return new DynamicMarkdownPreview(webview, input, contentProvider, previewConfigurations, logger, topmostLineMonitor, contributionProvider, engine);
    }
    static create(input, previewColumn, contentProvider, previewConfigurations, logger, topmostLineMonitor, contributionProvider, engine) {
        const webview = vscode.window.createWebviewPanel(DynamicMarkdownPreview.viewType, DynamicMarkdownPreview.getPreviewTitle(input.resource, input.locked), previewColumn, { enableFindWidget: true, });
        return new DynamicMarkdownPreview(webview, input, contentProvider, previewConfigurations, logger, topmostLineMonitor, contributionProvider, engine);
    }
    dispose() {
        this._preview.dispose();
        this._webviewPanel.dispose();
        this._onDisposeEmitter.fire();
        this._onDisposeEmitter.dispose();
        super.dispose();
    }
    get resource() {
        return this._preview.resource;
    }
    get resourceColumn() {
        return this._resourceColumn;
    }
    reveal(viewColumn) {
        this._webviewPanel.reveal(viewColumn);
    }
    refresh() {
        this._preview.refresh();
    }
    updateConfiguration() {
        if (this._previewConfigurations.hasConfigurationChanged(this._preview.resource)) {
            this.refresh();
        }
    }
    update(newResource, scrollLocation) {
        if (this._preview.isPreviewOf(newResource)) {
            switch (scrollLocation === null || scrollLocation === void 0 ? void 0 : scrollLocation.type) {
                case 'line':
                    this._preview.scrollTo(scrollLocation.line);
                    return;
                case 'fragment':
                    // Workaround. For fragments, just reload the entire preview
                    break;
                default:
                    return;
            }
        }
        this._preview.dispose();
        this._preview = this.createPreview(newResource, scrollLocation);
    }
    toggleLock() {
        this._locked = !this._locked;
        this._webviewPanel.title = DynamicMarkdownPreview.getPreviewTitle(this._preview.resource, this._locked);
    }
    static getPreviewTitle(resource, locked) {
        return locked
            ? localize('lockedPreviewTitle', '[Preview] {0}', path.basename(resource.fsPath))
            : localize('previewTitle', 'Preview {0}', path.basename(resource.fsPath));
    }
    get position() {
        return this._webviewPanel.viewColumn;
    }
    matchesResource(otherResource, otherPosition, otherLocked) {
        if (this.position !== otherPosition) {
            return false;
        }
        if (this._locked) {
            return otherLocked && this._preview.isPreviewOf(otherResource);
        }
        else {
            return !otherLocked;
        }
    }
    matches(otherPreview) {
        return this.matchesResource(otherPreview._preview.resource, otherPreview.position, otherPreview._locked);
    }
    createPreview(resource, startingScroll) {
        return new MarkdownPreview(this._webviewPanel, resource, startingScroll, {
            getTitle: (resource) => DynamicMarkdownPreview.getPreviewTitle(resource, this._locked),
            getAdditionalState: () => {
                return {
                    resourceColumn: this.resourceColumn,
                    locked: this._locked,
                };
            },
            openPreviewLinkToMarkdownFile: (link, fragment) => {
                this.update(link, fragment ? new StartingScrollFragment(fragment) : undefined);
            }
        }, this._engine, this._contentProvider, this._previewConfigurations, this._logger, this._contributionProvider);
    }
}
exports.DynamicMarkdownPreview = DynamicMarkdownPreview;
DynamicMarkdownPreview.viewType = 'markdown.preview';
//# sourceMappingURL=preview.js.map