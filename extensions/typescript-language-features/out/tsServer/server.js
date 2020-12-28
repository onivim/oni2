"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.GetErrRoutingTsServer = exports.SyntaxRoutingTsServer = exports.ProcessBasedTsServer = exports.PipeRequestCanceller = void 0;
const fs = require("fs");
const vscode = require("vscode");
const typescriptService_1 = require("../typescriptService");
const dispose_1 = require("../utils/dispose");
const wireProtocol_1 = require("../utils/wireProtocol");
const callbackMap_1 = require("./callbackMap");
const requestQueue_1 = require("./requestQueue");
const serverError_1 = require("./serverError");
class PipeRequestCanceller {
    constructor(_serverId, _cancellationPipeName, _tracer) {
        this._serverId = _serverId;
        this._cancellationPipeName = _cancellationPipeName;
        this._tracer = _tracer;
    }
    tryCancelOngoingRequest(seq) {
        if (!this._cancellationPipeName) {
            return false;
        }
        this._tracer.logTrace(this._serverId, `TypeScript Server: trying to cancel ongoing request with sequence number ${seq}`);
        try {
            fs.writeFileSync(this._cancellationPipeName + seq, '');
        }
        catch (_a) {
            // noop
        }
        return true;
    }
}
exports.PipeRequestCanceller = PipeRequestCanceller;
let ProcessBasedTsServer = /** @class */ (() => {
    class ProcessBasedTsServer extends dispose_1.Disposable {
        constructor(_serverId, _process, _tsServerLogFile, _requestCanceller, _version, _telemetryReporter, _tracer) {
            super();
            this._serverId = _serverId;
            this._process = _process;
            this._tsServerLogFile = _tsServerLogFile;
            this._requestCanceller = _requestCanceller;
            this._version = _version;
            this._telemetryReporter = _telemetryReporter;
            this._tracer = _tracer;
            this._requestQueue = new requestQueue_1.RequestQueue();
            this._callbacks = new callbackMap_1.CallbackMap();
            this._pendingResponses = new Set();
            this._onEvent = this._register(new vscode.EventEmitter());
            this.onEvent = this._onEvent.event;
            this._onExit = this._register(new vscode.EventEmitter());
            this.onExit = this._onExit.event;
            this._onError = this._register(new vscode.EventEmitter());
            this.onError = this._onError.event;
            this._reader = this._register(new wireProtocol_1.Reader(this._process.stdout));
            this._reader.onData(msg => this.dispatchMessage(msg));
            this._process.on('exit', code => {
                this._onExit.fire(code);
                this._callbacks.destroy('server exited');
            });
            this._process.on('error', error => {
                this._onError.fire(error);
                this._callbacks.destroy('server errored');
            });
        }
        get onReaderError() { return this._reader.onError; }
        get tsServerLogFile() { return this._tsServerLogFile; }
        write(serverRequest) {
            this._process.write(serverRequest);
        }
        dispose() {
            super.dispose();
            this._callbacks.destroy('server disposed');
            this._pendingResponses.clear();
        }
        kill() {
            this._process.kill();
        }
        dispatchMessage(message) {
            try {
                switch (message.type) {
                    case 'response':
                        this.dispatchResponse(message);
                        break;
                    case 'event':
                        const event = message;
                        if (event.event === 'requestCompleted') {
                            const seq = event.body.request_seq;
                            const callback = this._callbacks.fetch(seq);
                            if (callback) {
                                this._tracer.traceRequestCompleted(this._serverId, 'requestCompleted', seq, callback);
                                callback.onSuccess(undefined);
                            }
                        }
                        else {
                            this._tracer.traceEvent(this._serverId, event);
                            this._onEvent.fire(event);
                        }
                        break;
                    default:
                        throw new Error(`Unknown message type ${message.type} received`);
                }
            }
            finally {
                this.sendNextRequests();
            }
        }
        tryCancelRequest(seq, command) {
            try {
                if (this._requestQueue.tryDeletePendingRequest(seq)) {
                    this.logTrace(`Canceled request with sequence number ${seq}`);
                    return true;
                }
                if (this._requestCanceller.tryCancelOngoingRequest(seq)) {
                    return true;
                }
                this.logTrace(`Tried to cancel request with sequence number ${seq}. But request got already delivered.`);
                return false;
            }
            finally {
                const callback = this.fetchCallback(seq);
                if (callback) {
                    callback.onSuccess(new typescriptService_1.ServerResponse.Cancelled(`Cancelled request ${seq} - ${command}`));
                }
            }
        }
        dispatchResponse(response) {
            const callback = this.fetchCallback(response.request_seq);
            if (!callback) {
                return;
            }
            this._tracer.traceResponse(this._serverId, response, callback);
            if (response.success) {
                callback.onSuccess(response);
            }
            else if (response.message === 'No content available.') {
                // Special case where response itself is successful but there is not any data to return.
                callback.onSuccess(typescriptService_1.ServerResponse.NoContent);
            }
            else {
                callback.onError(serverError_1.TypeScriptServerError.create(this._serverId, this._version, response));
            }
        }
        executeImpl(command, args, executeInfo) {
            const request = this._requestQueue.createRequest(command, args);
            const requestInfo = {
                request,
                expectsResponse: executeInfo.expectsResult,
                isAsync: executeInfo.isAsync,
                queueingType: ProcessBasedTsServer.getQueueingType(command, executeInfo.lowPriority)
            };
            let result;
            if (executeInfo.expectsResult) {
                result = new Promise((resolve, reject) => {
                    this._callbacks.add(request.seq, { onSuccess: resolve, onError: reject, queuingStartTime: Date.now(), isAsync: executeInfo.isAsync }, executeInfo.isAsync);
                    if (executeInfo.token) {
                        executeInfo.token.onCancellationRequested(() => {
                            this.tryCancelRequest(request.seq, command);
                        });
                    }
                }).catch((err) => {
                    if (err instanceof serverError_1.TypeScriptServerError) {
                        if (!executeInfo.token || !executeInfo.token.isCancellationRequested) {
                            /* __GDPR__
                                "languageServiceErrorResponse" : {
                                    "${include}": [
                                        "${TypeScriptCommonProperties}",
                                        "${TypeScriptRequestErrorProperties}"
                                    ]
                                }
                            */
                            this._telemetryReporter.logTelemetry('languageServiceErrorResponse', err.telemetry);
                        }
                    }
                    throw err;
                });
            }
            this._requestQueue.enqueue(requestInfo);
            this.sendNextRequests();
            return result;
        }
        sendNextRequests() {
            while (this._pendingResponses.size === 0 && this._requestQueue.length > 0) {
                const item = this._requestQueue.dequeue();
                if (item) {
                    this.sendRequest(item);
                }
            }
        }
        sendRequest(requestItem) {
            const serverRequest = requestItem.request;
            this._tracer.traceRequest(this._serverId, serverRequest, requestItem.expectsResponse, this._requestQueue.length);
            if (requestItem.expectsResponse && !requestItem.isAsync) {
                this._pendingResponses.add(requestItem.request.seq);
            }
            try {
                this.write(serverRequest);
            }
            catch (err) {
                const callback = this.fetchCallback(serverRequest.seq);
                if (callback) {
                    callback.onError(err);
                }
            }
        }
        fetchCallback(seq) {
            const callback = this._callbacks.fetch(seq);
            if (!callback) {
                return undefined;
            }
            this._pendingResponses.delete(seq);
            return callback;
        }
        logTrace(message) {
            this._tracer.logTrace(this._serverId, message);
        }
        static getQueueingType(command, lowPriority) {
            if (ProcessBasedTsServer.fenceCommands.has(command)) {
                return requestQueue_1.RequestQueueingType.Fence;
            }
            return lowPriority ? requestQueue_1.RequestQueueingType.LowPriority : requestQueue_1.RequestQueueingType.Normal;
        }
    }
    ProcessBasedTsServer.fenceCommands = new Set(['change', 'close', 'open', 'updateOpen']);
    return ProcessBasedTsServer;
})();
exports.ProcessBasedTsServer = ProcessBasedTsServer;
let RequestRouter = /** @class */ (() => {
    class RequestRouter {
        constructor(servers, delegate) {
            this.servers = servers;
            this.delegate = delegate;
        }
        execute(command, args, executeInfo) {
            if (RequestRouter.sharedCommands.has(command)) {
                // Dispatch shared commands to all server but only return from first one one
                const requestStates = this.servers.map(() => RequestState.Unresolved);
                // Also make sure we never cancel requests to just one server
                let token = undefined;
                if (executeInfo.token) {
                    const source = new vscode.CancellationTokenSource();
                    executeInfo.token.onCancellationRequested(() => {
                        if (requestStates.some(state => state === RequestState.Resolved)) {
                            // Don't cancel.
                            // One of the servers completed this request so we don't want to leave the other
                            // in a different state.
                            return;
                        }
                        source.cancel();
                    });
                    token = source.token;
                }
                let firstRequest;
                for (let serverIndex = 0; serverIndex < this.servers.length; ++serverIndex) {
                    const server = this.servers[serverIndex].server;
                    const request = server.executeImpl(command, args, { ...executeInfo, token });
                    if (serverIndex === 0) {
                        firstRequest = request;
                    }
                    if (request) {
                        request
                            .then(result => {
                            requestStates[serverIndex] = RequestState.Resolved;
                            const erroredRequest = requestStates.find(state => state.type === 2 /* Errored */);
                            if (erroredRequest) {
                                // We've gone out of sync
                                this.delegate.onFatalError(command, erroredRequest.err);
                            }
                            return result;
                        }, err => {
                            requestStates[serverIndex] = new RequestState.Errored(err);
                            if (requestStates.some(state => state === RequestState.Resolved)) {
                                // We've gone out of sync
                                this.delegate.onFatalError(command, err);
                            }
                            throw err;
                        });
                    }
                }
                return firstRequest;
            }
            for (const { preferredCommands, server } of this.servers) {
                if (!preferredCommands || preferredCommands.has(command)) {
                    return server.executeImpl(command, args, executeInfo);
                }
            }
            throw new Error(`Could not find server for command: '${command}'`);
        }
    }
    RequestRouter.sharedCommands = new Set([
        'change',
        'close',
        'open',
        'updateOpen',
        'configure',
        'configurePlugin',
    ]);
    return RequestRouter;
})();
let SyntaxRoutingTsServer = /** @class */ (() => {
    class SyntaxRoutingTsServer extends dispose_1.Disposable {
        constructor(servers, delegate) {
            super();
            this._onEvent = this._register(new vscode.EventEmitter());
            this.onEvent = this._onEvent.event;
            this._onExit = this._register(new vscode.EventEmitter());
            this.onExit = this._onExit.event;
            this._onError = this._register(new vscode.EventEmitter());
            this.onError = this._onError.event;
            this.syntaxServer = servers.syntax;
            this.semanticServer = servers.semantic;
            this.router = new RequestRouter([
                { server: this.syntaxServer, preferredCommands: SyntaxRoutingTsServer.syntaxCommands },
                { server: this.semanticServer, preferredCommands: undefined /* gets all other commands */ }
            ], delegate);
            this._register(this.syntaxServer.onEvent(e => this._onEvent.fire(e)));
            this._register(this.semanticServer.onEvent(e => this._onEvent.fire(e)));
            this._register(this.semanticServer.onExit(e => {
                this._onExit.fire(e);
                this.syntaxServer.kill();
            }));
            this._register(this.semanticServer.onError(e => this._onError.fire(e)));
        }
        get onReaderError() { return this.semanticServer.onReaderError; }
        get tsServerLogFile() { return this.semanticServer.tsServerLogFile; }
        kill() {
            this.syntaxServer.kill();
            this.semanticServer.kill();
        }
        executeImpl(command, args, executeInfo) {
            return this.router.execute(command, args, executeInfo);
        }
    }
    SyntaxRoutingTsServer.syntaxCommands = new Set([
        'navtree',
        'getOutliningSpans',
        'jsxClosingTag',
        'selectionRange',
        'format',
        'formatonkey',
        'docCommentTemplate',
    ]);
    return SyntaxRoutingTsServer;
})();
exports.SyntaxRoutingTsServer = SyntaxRoutingTsServer;
let GetErrRoutingTsServer = /** @class */ (() => {
    class GetErrRoutingTsServer extends dispose_1.Disposable {
        constructor(servers, delegate) {
            super();
            this._onEvent = this._register(new vscode.EventEmitter());
            this.onEvent = this._onEvent.event;
            this._onExit = this._register(new vscode.EventEmitter());
            this.onExit = this._onExit.event;
            this._onError = this._register(new vscode.EventEmitter());
            this.onError = this._onError.event;
            this.getErrServer = servers.getErr;
            this.mainServer = servers.primary;
            this.router = new RequestRouter([
                { server: this.getErrServer, preferredCommands: new Set(['geterr', 'geterrForProject']) },
                { server: this.mainServer, preferredCommands: undefined /* gets all other commands */ }
            ], delegate);
            this._register(this.getErrServer.onEvent(e => {
                if (GetErrRoutingTsServer.diagnosticEvents.has(e.event)) {
                    this._onEvent.fire(e);
                }
                // Ignore all other events
            }));
            this._register(this.mainServer.onEvent(e => {
                if (!GetErrRoutingTsServer.diagnosticEvents.has(e.event)) {
                    this._onEvent.fire(e);
                }
                // Ignore all other events
            }));
            this._register(this.getErrServer.onError(e => this._onError.fire(e)));
            this._register(this.mainServer.onError(e => this._onError.fire(e)));
            this._register(this.mainServer.onExit(e => {
                this._onExit.fire(e);
                this.getErrServer.kill();
            }));
        }
        get onReaderError() { return this.mainServer.onReaderError; }
        get tsServerLogFile() { return this.mainServer.tsServerLogFile; }
        kill() {
            this.getErrServer.kill();
            this.mainServer.kill();
        }
        executeImpl(command, args, executeInfo) {
            return this.router.execute(command, args, executeInfo);
        }
    }
    GetErrRoutingTsServer.diagnosticEvents = new Set([
        'configFileDiag',
        'syntaxDiag',
        'semanticDiag',
        'suggestionDiag'
    ]);
    return GetErrRoutingTsServer;
})();
exports.GetErrRoutingTsServer = GetErrRoutingTsServer;
var RequestState;
(function (RequestState) {
    RequestState.Unresolved = { type: 0 /* Unresolved */ };
    RequestState.Resolved = { type: 1 /* Resolved */ };
    class Errored {
        constructor(err) {
            this.err = err;
            this.type = 2 /* Errored */;
        }
    }
    RequestState.Errored = Errored;
})(RequestState || (RequestState = {}));
//# sourceMappingURL=server.js.map