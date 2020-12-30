"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.joinPath = exports.normalizePath = exports.resolvePath = exports.isAbsolutePath = exports.basename = exports.dirname = exports.getScheme = exports.FileType = exports.serveFileSystemRequests = exports.FsReadDirRequest = exports.FsStatRequest = exports.FsContentRequest = void 0;
const vscode_1 = require("vscode");
const vscode_languageclient_1 = require("vscode-languageclient");
var FsContentRequest;
(function (FsContentRequest) {
    FsContentRequest.type = new vscode_languageclient_1.RequestType('fs/content');
})(FsContentRequest = exports.FsContentRequest || (exports.FsContentRequest = {}));
var FsStatRequest;
(function (FsStatRequest) {
    FsStatRequest.type = new vscode_languageclient_1.RequestType('fs/stat');
})(FsStatRequest = exports.FsStatRequest || (exports.FsStatRequest = {}));
var FsReadDirRequest;
(function (FsReadDirRequest) {
    FsReadDirRequest.type = new vscode_languageclient_1.RequestType('fs/readDir');
})(FsReadDirRequest = exports.FsReadDirRequest || (exports.FsReadDirRequest = {}));
function serveFileSystemRequests(client, runtime) {
    client.onRequest(FsContentRequest.type, (param) => {
        const uri = vscode_1.Uri.parse(param.uri);
        if (uri.scheme === 'file' && runtime.fs) {
            return runtime.fs.getContent(param.uri);
        }
        return vscode_1.workspace.fs.readFile(uri).then(buffer => {
            return new runtime.TextDecoder(param.encoding).decode(buffer);
        });
    });
    client.onRequest(FsReadDirRequest.type, (uriString) => {
        const uri = vscode_1.Uri.parse(uriString);
        if (uri.scheme === 'file' && runtime.fs) {
            return runtime.fs.readDirectory(uriString);
        }
        return vscode_1.workspace.fs.readDirectory(uri);
    });
    client.onRequest(FsStatRequest.type, (uriString) => {
        const uri = vscode_1.Uri.parse(uriString);
        if (uri.scheme === 'file' && runtime.fs) {
            return runtime.fs.stat(uriString);
        }
        return vscode_1.workspace.fs.stat(uri);
    });
}
exports.serveFileSystemRequests = serveFileSystemRequests;
var FileType;
(function (FileType) {
    /**
     * The file type is unknown.
     */
    FileType[FileType["Unknown"] = 0] = "Unknown";
    /**
     * A regular file.
     */
    FileType[FileType["File"] = 1] = "File";
    /**
     * A directory.
     */
    FileType[FileType["Directory"] = 2] = "Directory";
    /**
     * A symbolic link to a file.
     */
    FileType[FileType["SymbolicLink"] = 64] = "SymbolicLink";
})(FileType = exports.FileType || (exports.FileType = {}));
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