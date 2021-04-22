"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.joinPath = exports.normalizePath = exports.resolvePath = exports.isAbsolutePath = exports.extname = exports.basename = exports.dirname = exports.getScheme = exports.getRequestService = exports.FileType = exports.FsReadDirRequest = exports.FsStatRequest = exports.FsContentRequest = void 0;
const vscode_uri_1 = require("vscode-uri");
const vscode_languageserver_1 = require("vscode-languageserver");
var FsContentRequest;
(function (FsContentRequest) {
    FsContentRequest.type = new vscode_languageserver_1.RequestType('fs/content');
})(FsContentRequest = exports.FsContentRequest || (exports.FsContentRequest = {}));
var FsStatRequest;
(function (FsStatRequest) {
    FsStatRequest.type = new vscode_languageserver_1.RequestType('fs/stat');
})(FsStatRequest = exports.FsStatRequest || (exports.FsStatRequest = {}));
var FsReadDirRequest;
(function (FsReadDirRequest) {
    FsReadDirRequest.type = new vscode_languageserver_1.RequestType('fs/readDir');
})(FsReadDirRequest = exports.FsReadDirRequest || (exports.FsReadDirRequest = {}));
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
function getRequestService(handledSchemas, connection, runtime) {
    const builtInHandlers = {};
    for (let protocol of handledSchemas) {
        if (protocol === 'file') {
            builtInHandlers[protocol] = runtime.file;
        }
        else if (protocol === 'http' || protocol === 'https') {
            builtInHandlers[protocol] = runtime.http;
        }
    }
    return {
        async stat(uri) {
            const handler = builtInHandlers[getScheme(uri)];
            if (handler) {
                return handler.stat(uri);
            }
            const res = await connection.sendRequest(FsStatRequest.type, uri.toString());
            return res;
        },
        readDirectory(uri) {
            const handler = builtInHandlers[getScheme(uri)];
            if (handler) {
                return handler.readDirectory(uri);
            }
            return connection.sendRequest(FsReadDirRequest.type, uri.toString());
        },
        getContent(uri, encoding) {
            const handler = builtInHandlers[getScheme(uri)];
            if (handler) {
                return handler.getContent(uri, encoding);
            }
            return connection.sendRequest(FsContentRequest.type, { uri: uri.toString(), encoding });
        }
    };
}
exports.getRequestService = getRequestService;
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
function extname(uri) {
    for (let i = uri.length - 1; i >= 0; i--) {
        const ch = uri.charCodeAt(i);
        if (ch === Dot) {
            if (i > 0 && uri.charCodeAt(i - 1) !== Slash) {
                return uri.substr(i);
            }
            else {
                break;
            }
        }
        else if (ch === Slash) {
            break;
        }
    }
    return '';
}
exports.extname = extname;
function isAbsolutePath(path) {
    return path.charCodeAt(0) === Slash;
}
exports.isAbsolutePath = isAbsolutePath;
function resolvePath(uriString, path) {
    if (isAbsolutePath(path)) {
        const uri = vscode_uri_1.URI.parse(uriString);
        const parts = path.split('/');
        return uri.with({ path: normalizePath(parts) }).toString();
    }
    return joinPath(uriString, path);
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
function joinPath(uriString, ...paths) {
    const uri = vscode_uri_1.URI.parse(uriString);
    const parts = uri.path.split('/');
    for (let path of paths) {
        parts.push(...path.split('/'));
    }
    return uri.with({ path: normalizePath(parts) }).toString();
}
exports.joinPath = joinPath;
//# sourceMappingURL=requests.js.map