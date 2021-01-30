"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.MarkdownPreviewManager = void 0;
const vscode = require("vscode");
const dispose_1 = require("../util/dispose");
const topmostLineMonitor_1 = require("../util/topmostLineMonitor");
const preview_1 = require("./preview");
const previewConfig_1 = require("./previewConfig");
class PreviewStore extends dispose_1.Disposable {
    constructor() {
        super(...arguments);
        this._previews = new Set();
    }
    dispose() {
        super.dispose();
        for (const preview of this._previews) {
            preview.dispose();
        }
        this._previews.clear();
    }
    [Symbol.iterator]() {
        return this._previews[Symbol.iterator]();
    }
    get(resource, previewSettings) {
        for (const preview of this._previews) {
            if (preview.matchesResource(resource, previewSettings.previewColumn, previewSettings.locked)) {
                return preview;
            }
        }
        return undefined;
    }
    add(preview) {
        this._previews.add(preview);
    }
    delete(preview) {
        this._previews.delete(preview);
    }
}
class MarkdownPreviewManager extends dispose_1.Disposable {
    constructor(_contentProvider, _logger, _contributions, _engine) {
        super();
        this._contentProvider = _contentProvider;
        this._logger = _logger;
        this._contributions = _contributions;
        this._engine = _engine;
        this._topmostLineMonitor = new topmostLineMonitor_1.TopmostLineMonitor();
        this._previewConfigurations = new previewConfig_1.MarkdownPreviewConfigurationManager();
        this._dynamicPreviews = this._register(new PreviewStore());
        this._staticPreviews = this._register(new PreviewStore());
        this._activePreview = undefined;
        this.customEditorViewType = 'vscode.markdown.preview.editor';
        this._register(vscode.window.registerWebviewPanelSerializer(preview_1.DynamicMarkdownPreview.viewType, this));
        this._register(vscode.window.registerCustomEditorProvider(this.customEditorViewType, this));
    }
    refresh() {
        for (const preview of this._dynamicPreviews) {
            preview.refresh();
        }
        for (const preview of this._staticPreviews) {
            preview.refresh();
        }
    }
    updateConfiguration() {
        for (const preview of this._dynamicPreviews) {
            preview.updateConfiguration();
        }
        for (const preview of this._staticPreviews) {
            preview.updateConfiguration();
        }
    }
    openDynamicPreview(resource, settings) {
        let preview = this._dynamicPreviews.get(resource, settings);
        if (preview) {
            preview.reveal(settings.previewColumn);
        }
        else {
            preview = this.createNewDynamicPreview(resource, settings);
        }
        preview.update(resource);
    }
    get activePreviewResource() {
        var _a;
        return (_a = this._activePreview) === null || _a === void 0 ? void 0 : _a.resource;
    }
    get activePreviewResourceColumn() {
        var _a;
        return (_a = this._activePreview) === null || _a === void 0 ? void 0 : _a.resourceColumn;
    }
    toggleLock() {
        const preview = this._activePreview;
        if (preview instanceof preview_1.DynamicMarkdownPreview) {
            preview.toggleLock();
            // Close any previews that are now redundant, such as having two dynamic previews in the same editor group
            for (const otherPreview of this._dynamicPreviews) {
                if (otherPreview !== preview && preview.matches(otherPreview)) {
                    otherPreview.dispose();
                }
            }
        }
    }
    async deserializeWebviewPanel(webview, state) {
        const resource = vscode.Uri.parse(state.resource);
        const locked = state.locked;
        const line = state.line;
        const resourceColumn = state.resourceColumn;
        const preview = await preview_1.DynamicMarkdownPreview.revive({ resource, locked, line, resourceColumn }, webview, this._contentProvider, this._previewConfigurations, this._logger, this._topmostLineMonitor, this._contributions, this._engine);
        this.registerDynamicPreview(preview);
    }
    async resolveCustomTextEditor(document, webview) {
        const preview = preview_1.StaticMarkdownPreview.revive(document.uri, webview, this._contentProvider, this._previewConfigurations, this._logger, this._contributions, this._engine);
        this.registerStaticPreview(preview);
    }
    createNewDynamicPreview(resource, previewSettings) {
        const preview = preview_1.DynamicMarkdownPreview.create({
            resource,
            resourceColumn: previewSettings.resourceColumn,
            locked: previewSettings.locked,
        }, previewSettings.previewColumn, this._contentProvider, this._previewConfigurations, this._logger, this._topmostLineMonitor, this._contributions, this._engine);
        this.setPreviewActiveContext(true);
        this._activePreview = preview;
        return this.registerDynamicPreview(preview);
    }
    registerDynamicPreview(preview) {
        this._dynamicPreviews.add(preview);
        preview.onDispose(() => {
            this._dynamicPreviews.delete(preview);
        });
        this.trackActive(preview);
        preview.onDidChangeViewState(() => {
            // Remove other dynamic previews in our column
            dispose_1.disposeAll(Array.from(this._dynamicPreviews).filter(otherPreview => preview !== otherPreview && preview.matches(otherPreview)));
        });
        return preview;
    }
    registerStaticPreview(preview) {
        this._staticPreviews.add(preview);
        preview.onDispose(() => {
            this._staticPreviews.delete(preview);
        });
        this.trackActive(preview);
        return preview;
    }
    trackActive(preview) {
        preview.onDidChangeViewState(({ webviewPanel }) => {
            this.setPreviewActiveContext(webviewPanel.active);
            this._activePreview = webviewPanel.active ? preview : undefined;
        });
        preview.onDispose(() => {
            if (this._activePreview === preview) {
                this.setPreviewActiveContext(false);
                this._activePreview = undefined;
            }
        });
    }
    setPreviewActiveContext(value) {
        vscode.commands.executeCommand('setContext', MarkdownPreviewManager.markdownPreviewActiveContextKey, value);
    }
}
exports.MarkdownPreviewManager = MarkdownPreviewManager;
MarkdownPreviewManager.markdownPreviewActiveContextKey = 'markdownPreviewFocus';
//# sourceMappingURL=previewManager.js.map