"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.TypeScriptVersionProvider = exports.TypeScriptVersion = void 0;
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
const fs = require("fs");
const path = require("path");
const vscode = require("vscode");
const nls = require("vscode-nls");
const api_1 = require("./api");
const relativePathResolver_1 = require("./relativePathResolver");
const localize = nls.loadMessageBundle();
class TypeScriptVersion {
    constructor(source, path, _pathLabel) {
        this.source = source;
        this.path = path;
        this._pathLabel = _pathLabel;
        this.apiVersion = TypeScriptVersion.getApiVersion(this.tsServerPath);
    }
    get tsServerPath() {
        return path.join(this.path, 'tsserver.js');
    }
    get pathLabel() {
        var _a;
        return (_a = this._pathLabel) !== null && _a !== void 0 ? _a : this.path;
    }
    get isValid() {
        return this.apiVersion !== undefined;
    }
    eq(other) {
        if (this.path !== other.path) {
            return false;
        }
        if (this.apiVersion === other.apiVersion) {
            return true;
        }
        if (!this.apiVersion || !other.apiVersion) {
            return false;
        }
        return this.apiVersion.eq(other.apiVersion);
    }
    get displayName() {
        const version = this.apiVersion;
        return version ? version.displayName : localize('couldNotLoadTsVersion', 'Could not load the TypeScript version at this path');
    }
    static getApiVersion(serverPath) {
        const version = TypeScriptVersion.getTypeScriptVersion(serverPath);
        if (version) {
            return version;
        }
        // Allow TS developers to provide custom version
        const tsdkVersion = vscode.workspace.getConfiguration().get('typescript.tsdk_version', undefined);
        if (tsdkVersion) {
            return api_1.default.fromVersionString(tsdkVersion);
        }
        return undefined;
    }
    static getTypeScriptVersion(serverPath) {
        if (!fs.existsSync(serverPath)) {
            return undefined;
        }
        const p = serverPath.split(path.sep);
        if (p.length <= 2) {
            return undefined;
        }
        const p2 = p.slice(0, -2);
        const modulePath = p2.join(path.sep);
        let fileName = path.join(modulePath, 'package.json');
        if (!fs.existsSync(fileName)) {
            // Special case for ts dev versions
            if (path.basename(modulePath) === 'built') {
                fileName = path.join(modulePath, '..', 'package.json');
            }
        }
        if (!fs.existsSync(fileName)) {
            return undefined;
        }
        const contents = fs.readFileSync(fileName).toString();
        let desc = null;
        try {
            desc = JSON.parse(contents);
        }
        catch (err) {
            return undefined;
        }
        if (!desc || !desc.version) {
            return undefined;
        }
        return desc.version ? api_1.default.fromVersionString(desc.version) : undefined;
    }
}
exports.TypeScriptVersion = TypeScriptVersion;
class TypeScriptVersionProvider {
    constructor(configuration) {
        this.configuration = configuration;
    }
    updateConfiguration(configuration) {
        this.configuration = configuration;
    }
    get defaultVersion() {
        return this.globalVersion || this.bundledVersion;
    }
    get globalVersion() {
        if (this.configuration.globalTsdk) {
            const globals = this.loadVersionsFromSetting("user-setting" /* UserSetting */, this.configuration.globalTsdk);
            if (globals && globals.length) {
                return globals[0];
            }
        }
        return this.contributedTsNextVersion;
    }
    get localVersion() {
        const tsdkVersions = this.localTsdkVersions;
        if (tsdkVersions && tsdkVersions.length) {
            return tsdkVersions[0];
        }
        const nodeVersions = this.localNodeModulesVersions;
        if (nodeVersions && nodeVersions.length === 1) {
            return nodeVersions[0];
        }
        return undefined;
    }
    get localVersions() {
        const allVersions = this.localTsdkVersions.concat(this.localNodeModulesVersions);
        const paths = new Set();
        return allVersions.filter(x => {
            if (paths.has(x.path)) {
                return false;
            }
            paths.add(x.path);
            return true;
        });
    }
    get bundledVersion() {
        const version = this.getContributedVersion("bundled" /* Bundled */, 'vscode.typescript-language-features', ['..', 'node_modules']);
        if (version) {
            return version;
        }
        vscode.window.showErrorMessage(localize('noBundledServerFound', 'VS Code\'s tsserver was deleted by another application such as a misbehaving virus detection tool. Please reinstall VS Code.'));
        throw new Error('Could not find bundled tsserver.js');
    }
    get contributedTsNextVersion() {
        return this.getContributedVersion("ts-nightly-extension" /* TsNightlyExtension */, 'ms-vscode.vscode-typescript-next', ['node_modules']);
    }
    getContributedVersion(source, extensionId, pathToTs) {
        try {
            const extension = vscode.extensions.getExtension(extensionId);
            if (extension) {
                const typescriptPath = path.join(extension.extensionPath, ...pathToTs, 'typescript', 'lib');
                const bundledVersion = new TypeScriptVersion(source, typescriptPath, '');
                if (bundledVersion.isValid) {
                    return bundledVersion;
                }
            }
        }
        catch (_a) {
            // noop
        }
        return undefined;
    }
    get localTsdkVersions() {
        const localTsdk = this.configuration.localTsdk;
        return localTsdk ? this.loadVersionsFromSetting("workspace-setting" /* WorkspaceSetting */, localTsdk) : [];
    }
    loadVersionsFromSetting(source, tsdkPathSetting) {
        if (path.isAbsolute(tsdkPathSetting)) {
            return [new TypeScriptVersion(source, tsdkPathSetting)];
        }
        const workspacePath = relativePathResolver_1.RelativeWorkspacePathResolver.asAbsoluteWorkspacePath(tsdkPathSetting);
        if (workspacePath !== undefined) {
            return [new TypeScriptVersion(source, workspacePath, tsdkPathSetting)];
        }
        return this.loadTypeScriptVersionsFromPath(source, tsdkPathSetting);
    }
    get localNodeModulesVersions() {
        return this.loadTypeScriptVersionsFromPath("node-modules" /* NodeModules */, path.join('node_modules', 'typescript', 'lib'))
            .filter(x => x.isValid);
    }
    loadTypeScriptVersionsFromPath(source, relativePath) {
        if (!vscode.workspace.workspaceFolders) {
            return [];
        }
        const versions = [];
        for (const root of vscode.workspace.workspaceFolders) {
            let label = relativePath;
            if (vscode.workspace.workspaceFolders.length > 1) {
                label = path.join(root.name, relativePath);
            }
            versions.push(new TypeScriptVersion(source, path.join(root.uri.fsPath, relativePath), label));
        }
        return versions;
    }
}
exports.TypeScriptVersionProvider = TypeScriptVersionProvider;
//# sourceMappingURL=versionProvider.js.map