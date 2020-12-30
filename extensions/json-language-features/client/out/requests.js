"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.joinPath = exports.normalizePath = exports.resolvePath = exports.isAbsolutePath = exports.basename = exports.dirname = exports.getScheme = void 0;
function getScheme(uri) {
    return uri.substr(0, uri.indexOf(':'));
}
exports.getScheme = getScheme;
function dirname(uri) {
    const lastIndexOfSlash = uri.lastIndexOf('/');
    return lastIndexOfSlash !== -1 ? uri.substr(0, lastIndexOfSlash) : '';
}
exports.dirname = dirname;
function basename(uri) {
    const lastIndexOfSlash = uri.lastIndexOf('/');
    return uri.substr(lastIndexOfSlash + 1);
}
exports.basename = basename;
const Slash = '/'.charCodeAt(0);
const Dot = '.'.charCodeAt(0);
function isAbsolutePath(path) {
    return path.charCodeAt(0) === Slash;
}
exports.isAbsolutePath = isAbsolutePath;
function resolvePath(uri, path) {
    if (isAbsolutePath(path)) {
        return uri.with({ path: normalizePath(path.split('/')) });
    }
    return joinPath(uri, path);
}
exports.resolvePath = resolvePath;
function normalizePath(parts) {
    const newParts = [];
    for (const part of parts) {
        if (part.length === 0 || part.length === 1 && part.charCodeAt(0) === Dot) {
            // ignore
        }
        else if (part.length === 2 && part.charCodeAt(0) === Dot && part.charCodeAt(1) === Dot) {
            newParts.pop();
        }
        else {
            newParts.push(part);
        }
    }
    if (parts.length > 1 && parts[parts.length - 1].length === 0) {
        newParts.push('');
    }
    let res = newParts.join('/');
    if (parts[0].length === 0) {
        res = '/' + res;
    }
    return res;
}
exports.normalizePath = normalizePath;
function joinPath(uri, ...paths) {
    const parts = uri.path.split('/');
    for (let path of paths) {
        parts.push(...path.split('/'));
    }
    return uri.with({ path: normalizePath(parts) });
}
exports.joinPath = joinPath;
//# sourceMappingURL=requests.js.map