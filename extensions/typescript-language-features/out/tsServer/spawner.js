"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.TypeScriptServerSpawner = void 0;
const path = require("path");
const vscode = require("vscode");
const api_1 = require("../utils/api");
const configuration_1 = require("../utils/configuration");
const electron = require("../utils/electron");
const server_1 = require("./server");
class TypeScriptServerSpawner {
    constructor(_versionProvider, _logDirectoryProvider, _pluginPathsProvider, _logger, _telemetryReporter, _tracer) {
        this._versionProvider = _versionProvider;
        this._logDirectoryProvider = _logDirectoryProvider;
        this._pluginPathsProvider = _pluginPathsProvider;
        this._logger = _logger;
        this._telemetryReporter = _telemetryReporter;
        this._tracer = _tracer;
    }
    spawn(version, configuration, pluginManager, delegate) {
        let primaryServer;
        if (this.shouldUseSeparateSyntaxServer(version, configuration)) {
            primaryServer = new server_1.SyntaxRoutingTsServer({
                syntax: this.spawnTsServer("syntax" /* Syntax */, version, configuration, pluginManager),
                semantic: this.spawnTsServer("semantic" /* Semantic */, version, configuration, pluginManager)
            }, delegate);
        }
        else {
            primaryServer = this.spawnTsServer("main" /* Main */, version, configuration, pluginManager);
        }
        if (this.shouldUseSeparateDiagnosticsServer(configuration)) {
            return new server_1.GetErrRoutingTsServer({
                getErr: this.spawnTsServer("diagnostics" /* Diagnostics */, version, configuration, pluginManager),
                primary: primaryServer,
            }, delegate);
        }
        return primaryServer;
    }
    shouldUseSeparateSyntaxServer(version, configuration) {
        return configuration.useSeparateSyntaxServer && !!version.apiVersion && version.apiVersion.gte(api_1.default.v340);
    }
    shouldUseSeparateDiagnosticsServer(configuration) {
        return configuration.enableProjectDiagnostics;
    }
    spawnTsServer(kind, version, configuration, pluginManager) {
        const apiVersion = version.apiVersion || api_1.default.defaultVersion;
        const { args, cancellationPipeName, tsServerLogFile } = this.getTsServerArgs(kind, configuration, version, apiVersion, pluginManager);
        if (TypeScriptServerSpawner.isLoggingEnabled(configuration)) {
            if (tsServerLogFile) {
                this._logger.info(`<${kind}> Log file: ${tsServerLogFile}`);
            }
            else {
                this._logger.error(`<${kind}> Could not create log directory`);
            }
        }
        this._logger.info(`<${kind}> Forking...`);
        const childProcess = electron.fork(version.tsServerPath, args, this.getForkOptions(kind, configuration));
        this._logger.info(`<${kind}> Starting...`);
        return new server_1.ProcessBasedTsServer(kind, new ChildServerProcess(childProcess), tsServerLogFile, new server_1.PipeRequestCanceller(kind, cancellationPipeName, this._tracer), version, this._telemetryReporter, this._tracer);
    }
    getForkOptions(kind, configuration) {
        const debugPort = TypeScriptServerSpawner.getDebugPort(kind);
        const tsServerForkOptions = {
            execArgv: [
                ...(debugPort ? [`--inspect=${debugPort}`] : []),
                ...(configuration.maxTsServerMemory ? [`--max-old-space-size=${configuration.maxTsServerMemory}`] : [])
            ]
        };
        return tsServerForkOptions;
    }
    getTsServerArgs(kind, configuration, currentVersion, apiVersion, pluginManager) {
        const args = [];
        let tsServerLogFile;
        if (kind === "syntax" /* Syntax */) {
            args.push('--syntaxOnly');
        }
        if (apiVersion.gte(api_1.default.v250)) {
            args.push('--useInferredProjectPerProjectRoot');
        }
        else {
            args.push('--useSingleInferredProject');
        }
        if (configuration.disableAutomaticTypeAcquisition || kind === "syntax" /* Syntax */ || kind === "diagnostics" /* Diagnostics */) {
            args.push('--disableAutomaticTypingAcquisition');
        }
        if (kind === "semantic" /* Semantic */ || kind === "main" /* Main */) {
            args.push('--enableTelemetry');
        }
        const cancellationPipeName = electron.getTempFile('tscancellation');
        args.push('--cancellationPipeName', cancellationPipeName + '*');
        if (TypeScriptServerSpawner.isLoggingEnabled(configuration)) {
            const logDir = this._logDirectoryProvider.getNewLogDirectory();
            if (logDir) {
                tsServerLogFile = path.join(logDir, `tsserver.log`);
                args.push('--logVerbosity', configuration_1.TsServerLogLevel.toString(configuration.tsServerLogLevel));
                args.push('--logFile', tsServerLogFile);
            }
        }
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
        if (configuration.npmLocation) {
            args.push('--npmLocation', `"${configuration.npmLocation}"`);
        }
        if (apiVersion.gte(api_1.default.v260)) {
            args.push('--locale', TypeScriptServerSpawner.getTsLocale(configuration));
        }
        if (apiVersion.gte(api_1.default.v291)) {
            args.push('--noGetErrOnBackgroundUpdate');
        }
        if (apiVersion.gte(api_1.default.v345)) {
            args.push('--validateDefaultNpmLocation');
        }
        return { args, cancellationPipeName, tsServerLogFile };
    }
    static getDebugPort(kind) {
        if (kind === 'syntax') {
            // We typically only want to debug the main semantic server
            return undefined;
        }
        const value = process.env['TSS_DEBUG'];
        if (value) {
            const port = parseInt(value);
            if (!isNaN(port)) {
                return port;
            }
        }
        return undefined;
    }
    static isLoggingEnabled(configuration) {
        return configuration.tsServerLogLevel !== configuration_1.TsServerLogLevel.Off;
    }
    static getTsLocale(configuration) {
        return configuration.locale
            ? configuration.locale
            : vscode.env.language;
    }
}
exports.TypeScriptServerSpawner = TypeScriptServerSpawner;
class ChildServerProcess {
    constructor(_process) {
        this._process = _process;
    }
    get stdout() { return this._process.stdout; }
    write(serverRequest) {
        this._process.stdin.write(JSON.stringify(serverRequest) + '\r\n', 'utf8');
    }
    on(name, handler) {
        this._process.on(name, handler);
    }
    kill() {
        this._process.kill();
    }
}
//# sourceMappingURL=spawner.js.map