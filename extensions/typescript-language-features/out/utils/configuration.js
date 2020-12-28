"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.TypeScriptServiceConfiguration = exports.ImplicitProjectConfiguration = exports.TsServerLogLevel = void 0;
const os = require("os");
const path = require("path");
const vscode = require("vscode");
const objects = require("../utils/objects");
var TsServerLogLevel;
(function (TsServerLogLevel) {
    TsServerLogLevel[TsServerLogLevel["Off"] = 0] = "Off";
    TsServerLogLevel[TsServerLogLevel["Normal"] = 1] = "Normal";
    TsServerLogLevel[TsServerLogLevel["Terse"] = 2] = "Terse";
    TsServerLogLevel[TsServerLogLevel["Verbose"] = 3] = "Verbose";
})(TsServerLogLevel = exports.TsServerLogLevel || (exports.TsServerLogLevel = {}));
(function (TsServerLogLevel) {
    function fromString(value) {
        switch (value && value.toLowerCase()) {
            case 'normal':
                return TsServerLogLevel.Normal;
            case 'terse':
                return TsServerLogLevel.Terse;
            case 'verbose':
                return TsServerLogLevel.Verbose;
            case 'off':
            default:
                return TsServerLogLevel.Off;
        }
    }
    TsServerLogLevel.fromString = fromString;
    function toString(value) {
        switch (value) {
            case TsServerLogLevel.Normal:
                return 'normal';
            case TsServerLogLevel.Terse:
                return 'terse';
            case TsServerLogLevel.Verbose:
                return 'verbose';
            case TsServerLogLevel.Off:
            default:
                return 'off';
        }
    }
    TsServerLogLevel.toString = toString;
})(TsServerLogLevel = exports.TsServerLogLevel || (exports.TsServerLogLevel = {}));
class ImplicitProjectConfiguration {
    constructor(configuration) {
        this.checkJs = ImplicitProjectConfiguration.readCheckJs(configuration);
        this.experimentalDecorators = ImplicitProjectConfiguration.readExperimentalDecorators(configuration);
        this.strictNullChecks = ImplicitProjectConfiguration.readImplicitStrictNullChecks(configuration);
        this.strictFunctionTypes = ImplicitProjectConfiguration.readImplicitStrictFunctionTypes(configuration);
    }
    isEqualTo(other) {
        return objects.equals(this, other);
    }
    static readCheckJs(configuration) {
        var _a;
        return (_a = configuration.get('js/ts.implicitProjectConfig.checkJs')) !== null && _a !== void 0 ? _a : configuration.get('javascript.implicitProjectConfig.checkJs', false);
    }
    static readExperimentalDecorators(configuration) {
        var _a;
        return (_a = configuration.get('js/ts.implicitProjectConfig.experimentalDecorators')) !== null && _a !== void 0 ? _a : configuration.get('javascript.implicitProjectConfig.experimentalDecorators', false);
    }
    static readImplicitStrictNullChecks(configuration) {
        return configuration.get('js/ts.implicitProjectConfig.strictNullChecks', false);
    }
    static readImplicitStrictFunctionTypes(configuration) {
        return configuration.get('js/ts.implicitProjectConfig.strictFunctionTypes', true);
    }
}
exports.ImplicitProjectConfiguration = ImplicitProjectConfiguration;
class TypeScriptServiceConfiguration {
    constructor() {
        this.tsServerLogLevel = TsServerLogLevel.Off;
        const configuration = vscode.workspace.getConfiguration();
        this.locale = TypeScriptServiceConfiguration.extractLocale(configuration);
        this.globalTsdk = TypeScriptServiceConfiguration.extractGlobalTsdk(configuration);
        this.localTsdk = TypeScriptServiceConfiguration.extractLocalTsdk(configuration);
        this.npmLocation = TypeScriptServiceConfiguration.readNpmLocation(configuration);
        this.tsServerLogLevel = TypeScriptServiceConfiguration.readTsServerLogLevel(configuration);
        this.tsServerPluginPaths = TypeScriptServiceConfiguration.readTsServerPluginPaths(configuration);
        this.implictProjectConfiguration = new ImplicitProjectConfiguration(configuration);
        this.disableAutomaticTypeAcquisition = TypeScriptServiceConfiguration.readDisableAutomaticTypeAcquisition(configuration);
        this.separateSyntaxServer = TypeScriptServiceConfiguration.readUseSeparateSyntaxServer(configuration);
        this.enableProjectDiagnostics = TypeScriptServiceConfiguration.readEnableProjectDiagnostics(configuration);
        this.maxTsServerMemory = TypeScriptServiceConfiguration.readMaxTsServerMemory(configuration);
        this.enablePromptUseWorkspaceTsdk = TypeScriptServiceConfiguration.readEnablePromptUseWorkspaceTsdk(configuration);
        this.watchOptions = TypeScriptServiceConfiguration.readWatchOptions(configuration);
        this.includePackageJsonAutoImports = TypeScriptServiceConfiguration.readIncludePackageJsonAutoImports(configuration);
        this.enableTsServerTracing = TypeScriptServiceConfiguration.readEnableTsServerTracing(configuration);
    }
    static loadFromWorkspace() {
        return new TypeScriptServiceConfiguration();
    }
    isEqualTo(other) {
        return objects.equals(this, other);
    }
    static fixPathPrefixes(inspectValue) {
        const pathPrefixes = ['~' + path.sep];
        for (const pathPrefix of pathPrefixes) {
            if (inspectValue.startsWith(pathPrefix)) {
                return path.join(os.homedir(), inspectValue.slice(pathPrefix.length));
            }
        }
        return inspectValue;
    }
    static extractGlobalTsdk(configuration) {
        const inspect = configuration.inspect('typescript.tsdk');
        if (inspect && typeof inspect.globalValue === 'string') {
            return this.fixPathPrefixes(inspect.globalValue);
        }
        return null;
    }
    static extractLocalTsdk(configuration) {
        const inspect = configuration.inspect('typescript.tsdk');
        if (inspect && typeof inspect.workspaceValue === 'string') {
            return this.fixPathPrefixes(inspect.workspaceValue);
        }
        return null;
    }
    static readTsServerLogLevel(configuration) {
        const setting = configuration.get('typescript.tsserver.log', 'off');
        return TsServerLogLevel.fromString(setting);
    }
    static readTsServerPluginPaths(configuration) {
        return configuration.get('typescript.tsserver.pluginPaths', []);
    }
    static readNpmLocation(configuration) {
        return configuration.get('typescript.npm', null);
    }
    static readDisableAutomaticTypeAcquisition(configuration) {
        return configuration.get('typescript.disableAutomaticTypeAcquisition', false);
    }
    static extractLocale(configuration) {
        return configuration.get('typescript.locale', null);
    }
    static readUseSeparateSyntaxServer(configuration) {
        const value = configuration.get('typescript.tsserver.useSeparateSyntaxServer', true);
        if (value === true) {
            return 1 /* Enabled */;
        }
        return 0 /* Disabled */;
    }
    static readEnableProjectDiagnostics(configuration) {
        return configuration.get('typescript.tsserver.experimental.enableProjectDiagnostics', false);
    }
    static readWatchOptions(configuration) {
        return configuration.get('typescript.tsserver.watchOptions');
    }
    static readIncludePackageJsonAutoImports(configuration) {
        return configuration.get('typescript.preferences.includePackageJsonAutoImports');
    }
    static readMaxTsServerMemory(configuration) {
        const defaultMaxMemory = 3072;
        const minimumMaxMemory = 128;
        const memoryInMB = configuration.get('typescript.tsserver.maxTsServerMemory', defaultMaxMemory);
        if (!Number.isSafeInteger(memoryInMB)) {
            return defaultMaxMemory;
        }
        return Math.max(memoryInMB, minimumMaxMemory);
    }
    static readEnablePromptUseWorkspaceTsdk(configuration) {
        return configuration.get('typescript.enablePromptUseWorkspaceTsdk', false);
    }
    static readEnableTsServerTracing(configuration) {
        return configuration.get('typescript.tsserver.enableTracing', false);
    }
}
exports.TypeScriptServiceConfiguration = TypeScriptServiceConfiguration;
//# sourceMappingURL=configuration.js.map