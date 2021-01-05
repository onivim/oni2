"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.TypeScriptVersion = exports.localize = void 0;
const nls = require("vscode-nls");
exports.localize = nls.loadMessageBundle();
class TypeScriptVersion {
    constructor(source, path, apiVersion, _pathLabel) {
        this.source = source;
        this.path = path;
        this.apiVersion = apiVersion;
        this._pathLabel = _pathLabel;
    }
    get tsServerPath() {
        return this.path;
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
        return version ? version.displayName : exports.localize('couldNotLoadTsVersion', 'Could not load the TypeScript version at this path');
    }
}
exports.TypeScriptVersion = TypeScriptVersion;
//# sourceMappingURL=versionProvider.js.map