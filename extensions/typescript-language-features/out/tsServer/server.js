"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const fs = require("fs");
const path = require("path");
const vscode = require("vscode");
const typescriptService_1 = require("../typescriptService");
const api_1 = require("../utils/api");
const configuration_1 = require("../utils/configuration");
const dispose_1 = require("../utils/dispose");
const electron = require("../utils/electron");
const regexp_1 = require("../utils/regexp");
const wireProtocol_1 = require("../utils/wireProtocol");
const callbackMap_1 = require("./callbackMap");
const requestQueue_1 = require("./requestQueue");
class TypeScriptServerError extends Error {
    constructor(version, response, serverMessage, serverStack) {
        super(`TypeScript Server Error (${version.versionString})\n${serverMessage}\n${serverStack}`);
        this.response = response;
        this.serverMessage = serverMessage;
        this.serverStack = serverStack;
    }
    static create(version, response) {
        const parsedResult = TypeScriptServerError.parseErrorText(version, response);
        return new TypeScriptServerError(version, response, parsedResult ? parsedResult.message : undefined, parsedResult ? parsedResult.stack : undefined);
    }
    get serverErrorText() {
        return this.response.message;
    }
    get serverCommand() {
        return this.response.command;
    }
    /**
     * Given a `errorText` from a tsserver request indicating failure in handling a request,
     * prepares a payload for telemetry-logging.
     */
    static parseErrorText(version, response) {
        const errorText = response.message;
        if (errorText) {
            const errorPrefix = 'Error processing request. ';
            if (errorText.startsWith(errorPrefix)) {
                const prefixFreeErrorText = errorText.substr(errorPrefix.length);
                const newlineIndex = prefixFreeErrorText.indexOf('\n');
                if (newlineIndex >= 0) {
                    // Newline expected between message and stack.
                    return {
                        message: prefixFreeErrorText.substring(0, newlineIndex),
                        stack: TypeScriptServerError.normalizeMessageStack(version, prefixFreeErrorText.substring(newlineIndex + 1))
                    };
                }
            }
        }
        return undefined;
    }
    /**
     * Try to replace full TS Server paths with 'tsserver.js' so that we don't have to post process the data as much
     */
    static normalizeMessageStack(version, message) {
        if (!message) {
            return '';
        }
        return message.replace(new RegExp(`${regexp_1.escapeRegExp(version.path)}[/\\\\]tsserver.js:`, 'gi'), 'tsserver.js:');
    }
}
class TypeScriptServerSpawner {
    constructor(_versionProvider, _logDirectoryProvider, _pluginPathsProvider, _logger, _telemetryReporter, _tracer) {
        this._versionProvider = _versionProvider;
        this._logDirectoryProvider = _logDirectoryProvider;
        this._pluginPathsProvider = _pluginPathsProvider;
        this._logger = _logger;
        this._telemetryReporter = _telemetryReporter;
        this._tracer = _tracer;
    }
    spawn(version, configuration, pluginManager) {
        const apiVersion = version.version || api_1.default.defaultVersion;
        const { args, cancellationPipeName, tsServerLogFile } = this.getTsServerArgs(configuration, version, apiVersion, pluginManager);
        if (TypeScriptServerSpawner.isLoggingEnabled(apiVersion, configuration)) {
            if (tsServerLogFile) {
                this._logger.info(`TSServer log file: ${tsServerLogFile}`);
            }
            else {
                this._logger.error('Could not create TSServer log directory');
            }
        }
        this._logger.info('Forking TSServer');
        const childProcess = electron.fork(version.tsServerPath, args, this.getForkOptions());
        this._logger.info('Started TSServer');
        return new TypeScriptServer(childProcess, tsServerLogFile, cancellationPipeName, version, this._telemetryReporter, this._tracer);
    }
    getForkOptions() {
        const debugPort = TypeScriptServerSpawner.getDebugPort();
        const tsServerForkOptions = {
            execArgv: debugPort ? [`--inspect=${debugPort}`] : [],
        };
        return tsServerForkOptions;
    }
    getTsServerArgs(configuration, currentVersion, apiVersion, pluginManager) {
        const args = [];
        let cancellationPipeName;
        let tsServerLogFile;
        if (apiVersion.gte(api_1.default.v206)) {
            if (apiVersion.gte(api_1.default.v250)) {
                args.push('--useInferredProjectPerProjectRoot');
            }
            else {
                args.push('--useSingleInferredProject');
            }
            if (configuration.disableAutomaticTypeAcquisition) {
                args.push('--disableAutomaticTypingAcquisition');
            }
        }
        if (apiVersion.gte(api_1.default.v208)) {
            args.push('--enableTelemetry');
        }
        if (apiVersion.gte(api_1.default.v222)) {
            cancellationPipeName = electron.getTempFile('tscancellation');
            args.push('--cancellationPipeName', cancellationPipeName + '*');
        }
        if (TypeScriptServerSpawner.isLoggingEnabled(apiVersion, configuration)) {
            const logDir = this._logDirectoryProvider.getNewLogDirectory();
            if (logDir) {
                tsServerLogFile = path.join(logDir, `tsserver.log`);
                args.push('--logVerbosity', configuration_1.TsServerLogLevel.toString(configuration.tsServerLogLevel));
                args.push('--logFile', tsServerLogFile);
            }
        }
        if (apiVersion.gte(api_1.default.v230)) {
            const pluginPaths = this._pluginPathsProvider.getPluginPaths();
            if (pluginManager.plugins.length) {
                args.push('--globalPlugins', pluginManager.plugins.map(x => x.name).join(','));
                const isUsingBundledTypeScriptVersion = currentVersion.path === this._versionProvider.defaultVersion.path;
                for (const plugin of pluginManager.plugins) {
                    if (isUsingBundledTypeScriptVersion || plugin.enableForWorkspaceTypeScriptVersions) {
                        pluginPaths.push(plugin.path);
                    }
                }
            }
            if (pluginPaths.length !== 0) {
                args.push('--pluginProbeLocations', pluginPaths.join(','));
            }
        }
        if (apiVersion.gte(api_1.default.v234)) {
            if (configuration.npmLocation) {
                args.push('--npmLocation', `"${configuration.npmLocation}"`);
            }
        }
        if (apiVersion.gte(api_1.default.v260)) {
            args.push('--locale', TypeScriptServerSpawner.getTsLocale(configuration));
        }
        if (apiVersion.gte(api_1.default.v291)) {
            args.push('--noGetErrOnBackgroundUpdate');
        }
        return { args, cancellationPipeName, tsServerLogFile };
    }
    static getDebugPort() {
        const value = process.env['TSS_DEBUG'];
        if (value) {
            const port = parseInt(value);
            if (!isNaN(port)) {
                return port;
            }
        }
        return undefined;
    }
    static isLoggingEnabled(apiVersion, configuration) {
        return apiVersion.gte(api_1.default.v222) &&
            configuration.tsServerLogLevel !== configuration_1.TsServerLogLevel.Off;
    }
    static getTsLocale(configuration) {
        return configuration.locale
            ? configuration.locale
            : vscode.env.language;
    }
}
exports.TypeScriptServerSpawner = TypeScriptServerSpawner;
class TypeScriptServer extends dispose_1.Disposable {
    constructor(_childProcess, _tsServerLogFile, _cancellationPipeName, _version, _telemetryReporter, _tracer) {
        super();
        this._childProcess = _childProcess;
        this._tsServerLogFile = _tsServerLogFile;
        this._cancellationPipeName = _cancellationPipeName;
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
        this._reader = this._register(new wireProtocol_1.Reader(this._childProcess.stdout));
        this._reader.onData(msg => this.dispatchMessage(msg));
        this._childProcess.on('exit', code => this.handleExit(code));
        this._childProcess.on('error', error => this.handleError(error));
    }
    get onReaderError() { return this._reader.onError; }
    get tsServerLogFile() { return this._tsServerLogFile; }
    write(serverRequest) {
        this._childProcess.stdin.write(JSON.stringify(serverRequest) + '\r\n', 'utf8');
    }
    dispose() {
        super.dispose();
        this._callbacks.destroy('server disposed');
        this._pendingResponses.clear();
    }
    kill() {
        this._childProcess.kill();
    }
    handleExit(error) {
        this._onExit.fire(error);
        this._callbacks.destroy('server exited');
    }
    handleError(error) {
        this._onError.fire(error);
        this._callbacks.destroy('server errored');
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
                        const p = this._callbacks.fetch(seq);
                        if (p) {
                            this._tracer.traceRequestCompleted('requestCompleted', seq, p.startTime);
                            p.onSuccess(undefined);
                        }
                    }
                    else {
                        this._tracer.traceEvent(event);
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
                this._tracer.logTrace(`TypeScript Server: canceled request with sequence number ${seq}`);
                return true;
            }
            if (this._cancellationPipeName) {
                this._tracer.logTrace(`TypeScript Server: trying to cancel ongoing request with sequence number ${seq}`);
                try {
                    fs.writeFileSync(this._cancellationPipeName + seq, '');
                }
                catch (_a) {
                    // noop
                }
                return true;
            }
            this._tracer.logTrace(`TypeScript Server: tried to cancel request with sequence number ${seq}. But request got already delivered.`);
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
        this._tracer.traceResponse(response, callback.startTime);
        if (response.success) {
            callback.onSuccess(response);
        }
        else if (response.message === 'No content available.') {
            // Special case where response itself is successful but there is not any data to return.
            callback.onSuccess(typescriptService_1.ServerResponse.NoContent);
        }
        else {
            callback.onError(TypeScriptServerError.create(this._version, response));
        }
    }
    executeImpl(command, args, executeInfo) {
        const request = this._requestQueue.createRequest(command, args);
        const requestInfo = {
            request,
            expectsResponse: executeInfo.expectsResult,
            isAsync: executeInfo.isAsync,
            queueingType: getQueueingType(command, executeInfo.lowPriority)
        };
        let result;
        if (executeInfo.expectsResult) {
            result = new Promise((resolve, reject) => {
                this._callbacks.add(request.seq, { onSuccess: resolve, onError: reject, startTime: Date.now(), isAsync: executeInfo.isAsync }, executeInfo.isAsync);
                if (executeInfo.token) {
                    executeInfo.token.onCancellationRequested(() => {
                        this.tryCancelRequest(request.seq, command);
                    });
                }
            }).catch((err) => {
                if (err instanceof TypeScriptServerError) {
                    if (!executeInfo.token || !executeInfo.token.isCancellationRequested) {
                        /* __GDPR__
                            "languageServiceErrorResponse" : {
                                "command" : { "classification": "SystemMetaData", "purpose": "FeatureInsight" },
                                "message" : { "classification": "CallstackOrException", "purpose": "PerformanceAndHealth" },
                                "stack" : { "classification": "CallstackOrException", "purpose": "PerformanceAndHealth" },
                                "errortext" : { "classification": "CallstackOrException", "purpose": "PerformanceAndHealth" },
                                "${include}": [
                                    "${TypeScriptCommonProperties}"
                                ]
                            }
                        */
                        this._telemetryReporter.logTelemetry('languageServiceErrorResponse', {
                            command: err.serverCommand,
                            message: err.serverMessage || '',
                            stack: err.serverStack || '',
                            errortext: err.serverErrorText || '',
                        });
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
        this._tracer.traceRequest(serverRequest, requestItem.expectsResponse, this._requestQueue.length);
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
}
exports.TypeScriptServer = TypeScriptServer;
const fenceCommands = new Set(['change', 'close', 'open']);
function getQueueingType(command, lowPriority) {
    if (fenceCommands.has(command)) {
        return requestQueue_1.RequestQueueingType.Fence;
    }
    return lowPriority ? requestQueue_1.RequestQueueingType.LowPriority : requestQueue_1.RequestQueueingType.Normal;
}
//# sourceMappingURL=server.js.map