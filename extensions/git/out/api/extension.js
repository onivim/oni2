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
const api1_1 = require("./api1");
const vscode_1 = require("vscode");
const util_1 = require("../util");
function deprecated(_target, key, descriptor) {
    if (typeof descriptor.value !== 'function') {
        throw new Error('not supported');
    }
    const fn = descriptor.value;
    descriptor.value = function () {
        console.warn(`Git extension API method '${key}' is deprecated.`);
        return fn.apply(this, arguments);
    };
}
exports.deprecated = deprecated;
class GitExtensionImpl {
    constructor(model) {
        this.enabled = false;
        this._onDidChangeEnablement = new vscode_1.EventEmitter();
        this.onDidChangeEnablement = util_1.latchEvent(this._onDidChangeEnablement.event);
        this._model = undefined;
        if (model) {
            this.enabled = true;
            this._model = model;
        }
    }
    set model(model) {
        this._model = model;
        this.enabled = !!model;
        this._onDidChangeEnablement.fire(this.enabled);
    }
    async getGitPath() {
        if (!this._model) {
            throw new Error('Git model not found');
        }
        return this._model.git.path;
    }
    async getRepositories() {
        if (!this._model) {
            throw new Error('Git model not found');
        }
        return this._model.repositories.map(repository => new api1_1.ApiRepository(repository));
    }
    getAPI(version) {
        if (!this._model) {
            throw new Error('Git model not found');
        }
        if (version !== 1) {
            throw new Error(`No API version ${version} found.`);
        }
        return new api1_1.ApiImpl(this._model);
    }
}
__decorate([
    deprecated
], GitExtensionImpl.prototype, "getGitPath", null);
__decorate([
    deprecated
], GitExtensionImpl.prototype, "getRepositories", null);
exports.GitExtensionImpl = GitExtensionImpl;
//# sourceMappingURL=extension.js.map