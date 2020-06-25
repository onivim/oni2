"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const vscode = require("vscode");
const api_1 = require("../utils/api");
const arrays_1 = require("../utils/arrays");
const async_1 = require("../utils/async");
const cancellation_1 = require("../utils/cancellation");
const dispose_1 = require("../utils/dispose");
const languageModeIds = require("../utils/languageModeIds");
const resourceMap_1 = require("../utils/resourceMap");
const typeConverters = require("../utils/typeConverters");
function mode2ScriptKind(mode) {
    switch (mode) {
        case languageModeIds.typescript: return 'TS';
        case languageModeIds.typescriptreact: return 'TSX';
        case languageModeIds.javascript: return 'JS';
        case languageModeIds.javascriptreact: return 'JSX';
    }
    return undefined;
}
class CloseOperation {
    constructor(args) {
        this.args = args;
        this.type = 0 /* Close */;
    }
}
class OpenOperation {
    constructor(args) {
        this.args = args;
        this.type = 1 /* Open */;
    }
}
class ChangeOperation {
    constructor(args) {
        this.args = args;
        this.type = 2 /* Change */;
    }
}
/**
 * Manages synchronization of buffers with the TS server.
 *
 * If supported, batches together file changes. This allows the TS server to more efficiently process changes.
 */
class BufferSynchronizer {
    constructor(client) {
        this.client = client;
        this._pending = new resourceMap_1.ResourceMap();
    }
    open(resource, args) {
        if (this.supportsBatching) {
            this.updatePending(resource, new OpenOperation(args));
        }
        else {
            this.client.executeWithoutWaitingForResponse('open', args);
        }
    }
    close(resource, filepath) {
        if (this.supportsBatching) {
            this.updatePending(resource, new CloseOperation(filepath));
        }
        else {
            const args = { file: filepath };
            this.client.executeWithoutWaitingForResponse('close', args);
        }
    }
    change(resource, filepath, events) {
        if (!events.length) {
            return;
        }
        if (this.supportsBatching) {
            this.updatePending(resource, new ChangeOperation({
                fileName: filepath,
                textChanges: events.map((change) => ({
                    newText: change.text,
                    start: typeConverters.Position.toLocation(change.range.start),
                    end: typeConverters.Position.toLocation(change.range.end),
                })).reverse(),
            }));
        }
        else {
            for (const { range, text } of events) {
                const args = {
                    insertString: text,
                    ...typeConverters.Range.toFormattingRequestArgs(filepath, range)
                };
                this.client.executeWithoutWaitingForResponse('change', args);
            }
        }
    }
    reset() {
        this._pending.clear();
    }
    beforeCommand(command) {
        if (command === 'updateOpen') {
            return;
        }
        this.flush();
    }
    flush() {
        if (!this.supportsBatching) {
            // We've already eagerly synchronized
            this._pending.clear();
            return;
        }
        if (this._pending.size > 0) {
            const closedFiles = [];
            const openFiles = [];
            const changedFiles = [];
            for (const change of this._pending.values) {
                switch (change.type) {
                    case 2 /* Change */:
                        changedFiles.push(change.args);
                        break;
                    case 1 /* Open */:
                        openFiles.push(change.args);
                        break;
                    case 0 /* Close */:
                        closedFiles.push(change.args);
                        break;
                }
            }
            this.client.execute('updateOpen', { changedFiles, closedFiles, openFiles }, cancellation_1.nulToken, { nonRecoverable: true });
            this._pending.clear();
        }
    }
    get supportsBatching() {
        return this.client.apiVersion.gte(api_1.default.v340);
    }
    updatePending(resource, op) {
        switch (op.type) {
            case 0 /* Close */:
                const existing = this._pending.get(resource);
                switch (existing === null || existing === void 0 ? void 0 : existing.type) {
                    case 1 /* Open */:
                        this._pending.delete(resource);
                        return; // Open then close. No need to do anything
                }
                break;
        }
        if (this._pending.has(resource)) {
            // we saw this file before, make sure we flush before working with it again
            this.flush();
        }
        this._pending.set(resource, op);
    }
}
class SyncedBuffer {
    constructor(document, filepath, client, synchronizer) {
        this.document = document;
        this.filepath = filepath;
        this.client = client;
        this.synchronizer = synchronizer;
        this.state = 1 /* Initial */;
    }
    open() {
        const args = {
            file: this.filepath,
            fileContent: this.document.getText(),
            projectRootPath: this.client.getWorkspaceRootForResource(this.document.uri),
        };
        const scriptKind = mode2ScriptKind(this.document.languageId);
        if (scriptKind) {
            args.scriptKindName = scriptKind;
        }
        if (this.client.apiVersion.gte(api_1.default.v240)) {
            const tsPluginsForDocument = this.client.pluginManager.plugins
                .filter(x => x.languages.indexOf(this.document.languageId) >= 0);
            if (tsPluginsForDocument.length) {
                args.plugins = tsPluginsForDocument.map(plugin => plugin.name);
            }
        }
        this.synchronizer.open(this.resource, args);
        this.state = 2 /* Open */;
    }
    get resource() {
        return this.document.uri;
    }
    get lineCount() {
        return this.document.lineCount;
    }
    get kind() {
        switch (this.document.languageId) {
            case languageModeIds.javascript:
            case languageModeIds.javascriptreact:
                return 2 /* JavaScript */;
            case languageModeIds.typescript:
            case languageModeIds.typescriptreact:
            default:
                return 1 /* TypeScript */;
        }
    }
    close() {
        this.synchronizer.close(this.resource, this.filepath);
        this.state = 2 /* Closed */;
    }
    onContentChanged(events) {
        if (this.state !== 2 /* Open */) {
            console.error(`Unexpected buffer state: ${this.state}`);
        }
        this.synchronizer.change(this.resource, this.filepath, events);
    }
}
class SyncedBufferMap extends resourceMap_1.ResourceMap {
    getForPath(filePath) {
        return this.get(vscode.Uri.file(filePath));
    }
    get allBuffers() {
        return this.values;
    }
}
class PendingDiagnostics extends resourceMap_1.ResourceMap {
    getOrderedFileSet() {
        const orderedResources = Array.from(this.entries)
            .sort((a, b) => a.value - b.value)
            .map(entry => entry.resource);
        const map = new resourceMap_1.ResourceMap();
        for (const resource of orderedResources) {
            map.set(resource, undefined);
        }
        return map;
    }
}
class GetErrRequest {
    constructor(client, files, onDone) {
        this.files = files;
        this._done = false;
        this._token = new vscode.CancellationTokenSource();
        const allFiles = arrays_1.coalesce(Array.from(files.entries).map(entry => client.normalizedPath(entry.resource)));
        if (!allFiles.length) {
            this._done = true;
            onDone();
        }
        else {
            const request = client.configuration.enableProjectDiagnostics
                // Note that geterrForProject is almost certainly not the api we want here as it ends up computing far
                // too many diagnostics
                ? client.executeAsync('geterrForProject', { delay: 0, file: allFiles[0] }, this._token.token)
                : client.executeAsync('geterr', { delay: 0, files: allFiles }, this._token.token);
            request.finally(() => {
                if (this._done) {
                    return;
                }
                this._done = true;
                onDone();
            });
        }
    }
    static executeGetErrRequest(client, files, onDone) {
        return new GetErrRequest(client, files, onDone);
    }
    cancel() {
        if (!this._done) {
            this._token.cancel();
        }
        this._token.dispose();
    }
}
class BufferSyncSupport extends dispose_1.Disposable {
    constructor(client, modeIds) {
        super();
        this._validateJavaScript = true;
        this._validateTypeScript = true;
        this.listening = false;
        this._onDelete = this._register(new vscode.EventEmitter());
        this.onDelete = this._onDelete.event;
        this._onWillChange = this._register(new vscode.EventEmitter());
        this.onWillChange = this._onWillChange.event;
        this.client = client;
        this.modeIds = new Set(modeIds);
        this.diagnosticDelayer = new async_1.Delayer(300);
        const pathNormalizer = (path) => this.client.normalizedPath(path);
        this.syncedBuffers = new SyncedBufferMap(pathNormalizer);
        this.pendingDiagnostics = new PendingDiagnostics(pathNormalizer);
        this.synchronizer = new BufferSynchronizer(client);
        this.updateConfiguration();
        vscode.workspace.onDidChangeConfiguration(this.updateConfiguration, this, this._disposables);
    }
    listen() {
        if (this.listening) {
            return;
        }
        this.listening = true;
        vscode.workspace.onDidOpenTextDocument(this.openTextDocument, this, this._disposables);
        vscode.workspace.onDidCloseTextDocument(this.onDidCloseTextDocument, this, this._disposables);
        vscode.workspace.onDidChangeTextDocument(this.onDidChangeTextDocument, this, this._disposables);
        vscode.window.onDidChangeVisibleTextEditors(e => {
            for (const { document } of e) {
                const syncedBuffer = this.syncedBuffers.get(document.uri);
                if (syncedBuffer) {
                    this.requestDiagnostic(syncedBuffer);
                }
            }
        }, this, this._disposables);
        vscode.workspace.textDocuments.forEach(this.openTextDocument, this);
    }
    handles(resource) {
        return this.syncedBuffers.has(resource);
    }
    ensureHasBuffer(resource) {
        if (this.syncedBuffers.has(resource)) {
            return true;
        }
        const existingDocument = vscode.workspace.textDocuments.find(doc => doc.uri.toString() === resource.toString());
        if (existingDocument) {
            return this.openTextDocument(existingDocument);
        }
        return false;
    }
    toVsCodeResource(resource) {
        const filepath = this.client.normalizedPath(resource);
        for (const buffer of this.syncedBuffers.allBuffers) {
            if (buffer.filepath === filepath) {
                return buffer.resource;
            }
        }
        return resource;
    }
    toResource(filePath) {
        const buffer = this.syncedBuffers.getForPath(filePath);
        if (buffer) {
            return buffer.resource;
        }
        return vscode.Uri.file(filePath);
    }
    reset() {
        var _a;
        (_a = this.pendingGetErr) === null || _a === void 0 ? void 0 : _a.cancel();
        this.pendingDiagnostics.clear();
        this.synchronizer.reset();
    }
    reinitialize() {
        this.reset();
        for (const buffer of this.syncedBuffers.allBuffers) {
            buffer.open();
        }
    }
    openTextDocument(document) {
        if (!this.modeIds.has(document.languageId)) {
            return false;
        }
        const resource = document.uri;
        const filepath = this.client.normalizedPath(resource);
        if (!filepath) {
            return false;
        }
        if (this.syncedBuffers.has(resource)) {
            return true;
        }
        const syncedBuffer = new SyncedBuffer(document, filepath, this.client, this.synchronizer);
        this.syncedBuffers.set(resource, syncedBuffer);
        syncedBuffer.open();
        this.requestDiagnostic(syncedBuffer);
        return true;
    }
    closeResource(resource) {
        var _a;
        const syncedBuffer = this.syncedBuffers.get(resource);
        if (!syncedBuffer) {
            return;
        }
        this.pendingDiagnostics.delete(resource);
        (_a = this.pendingGetErr) === null || _a === void 0 ? void 0 : _a.files.delete(resource);
        this.syncedBuffers.delete(resource);
        syncedBuffer.close();
        this._onDelete.fire(resource);
        this.requestAllDiagnostics();
    }
    interuptGetErr(f) {
        if (!this.pendingGetErr
            || this.client.configuration.enableProjectDiagnostics // `geterr` happens on seperate server so no need to cancel it.
        ) {
            return f();
        }
        this.pendingGetErr.cancel();
        this.pendingGetErr = undefined;
        const result = f();
        this.triggerDiagnostics();
        return result;
    }
    beforeCommand(command) {
        this.synchronizer.beforeCommand(command);
    }
    onDidCloseTextDocument(document) {
        this.closeResource(document.uri);
    }
    onDidChangeTextDocument(e) {
        const syncedBuffer = this.syncedBuffers.get(e.document.uri);
        if (!syncedBuffer) {
            return;
        }
        this._onWillChange.fire(syncedBuffer.resource);
        syncedBuffer.onContentChanged(e.contentChanges);
        const didTrigger = this.requestDiagnostic(syncedBuffer);
        if (!didTrigger && this.pendingGetErr) {
            // In this case we always want to re-trigger all diagnostics
            this.pendingGetErr.cancel();
            this.pendingGetErr = undefined;
            this.triggerDiagnostics();
        }
    }
    requestAllDiagnostics() {
        for (const buffer of this.syncedBuffers.allBuffers) {
            if (this.shouldValidate(buffer)) {
                this.pendingDiagnostics.set(buffer.resource, Date.now());
            }
        }
        this.triggerDiagnostics();
    }
    getErr(resources) {
        const handledResources = resources.filter(resource => this.handles(resource));
        if (!handledResources.length) {
            return;
        }
        for (const resource of handledResources) {
            this.pendingDiagnostics.set(resource, Date.now());
        }
        this.triggerDiagnostics();
    }
    triggerDiagnostics(delay = 200) {
        this.diagnosticDelayer.trigger(() => {
            this.sendPendingDiagnostics();
        }, delay);
    }
    requestDiagnostic(buffer) {
        if (!this.shouldValidate(buffer)) {
            return false;
        }
        this.pendingDiagnostics.set(buffer.resource, Date.now());
        const delay = Math.min(Math.max(Math.ceil(buffer.lineCount / 20), 300), 800);
        this.triggerDiagnostics(delay);
        return true;
    }
    hasPendingDiagnostics(resource) {
        return this.pendingDiagnostics.has(resource);
    }
    sendPendingDiagnostics() {
        const orderedFileSet = this.pendingDiagnostics.getOrderedFileSet();
        if (this.pendingGetErr) {
            this.pendingGetErr.cancel();
            for (const { resource } of this.pendingGetErr.files.entries) {
                if (this.syncedBuffers.get(resource)) {
                    orderedFileSet.set(resource, undefined);
                }
            }
            this.pendingGetErr = undefined;
        }
        // Add all open TS buffers to the geterr request. They might be visible
        for (const buffer of this.syncedBuffers.values) {
            orderedFileSet.set(buffer.resource, undefined);
        }
        if (orderedFileSet.size) {
            const getErr = this.pendingGetErr = GetErrRequest.executeGetErrRequest(this.client, orderedFileSet, () => {
                if (this.pendingGetErr === getErr) {
                    this.pendingGetErr = undefined;
                }
            });
        }
        this.pendingDiagnostics.clear();
    }
    updateConfiguration() {
        const jsConfig = vscode.workspace.getConfiguration('javascript', null);
        const tsConfig = vscode.workspace.getConfiguration('typescript', null);
        this._validateJavaScript = jsConfig.get('validate.enable', true);
        this._validateTypeScript = tsConfig.get('validate.enable', true);
    }
    shouldValidate(buffer) {
        switch (buffer.kind) {
            case 2 /* JavaScript */:
                return this._validateJavaScript;
            case 1 /* TypeScript */:
            default:
                return this._validateTypeScript;
        }
    }
}
exports.default = BufferSyncSupport;
//# sourceMappingURL=bufferSyncSupport.js.map