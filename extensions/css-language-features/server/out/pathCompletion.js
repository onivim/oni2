"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const path = require("path");
const fs = require("fs");
const vscode_uri_1 = require("vscode-uri");
const vscode_languageserver_types_1 = require("vscode-languageserver-types");
const strings_1 = require("./utils/strings");
function getPathCompletionParticipant(document, workspaceFolders, result) {
    return {
        onCssURILiteralValue: ({ position, range, uriValue }) => {
            const fullValue = stripQuotes(uriValue);
            if (!shouldDoPathCompletion(uriValue, workspaceFolders)) {
                if (fullValue === '.' || fullValue === '..') {
                    result.isIncomplete = true;
                }
                return;
            }
            let suggestions = providePathSuggestions(uriValue, position, range, document, workspaceFolders);
            result.items = [...suggestions, ...result.items];
        },
        onCssImportPath: ({ position, range, pathValue }) => {
            const fullValue = stripQuotes(pathValue);
            if (!shouldDoPathCompletion(pathValue, workspaceFolders)) {
                if (fullValue === '.' || fullValue === '..') {
                    result.isIncomplete = true;
                }
                return;
            }
            let suggestions = providePathSuggestions(pathValue, position, range, document, workspaceFolders);
            if (document.languageId === 'scss') {
                suggestions.forEach(s => {
                    if (strings_1.startsWith(s.label, '_') && strings_1.endsWith(s.label, '.scss')) {
                        if (s.textEdit) {
                            s.textEdit.newText = s.label.slice(1, -5);
                        }
                        else {
                            s.label = s.label.slice(1, -5);
                        }
                    }
                });
            }
            result.items = [...suggestions, ...result.items];
        }
    };
}
exports.getPathCompletionParticipant = getPathCompletionParticipant;
function providePathSuggestions(pathValue, position, range, document, workspaceFolders) {
    const fullValue = stripQuotes(pathValue);
    const isValueQuoted = strings_1.startsWith(pathValue, `'`) || strings_1.startsWith(pathValue, `"`);
    const valueBeforeCursor = isValueQuoted
        ? fullValue.slice(0, position.character - (range.start.character + 1))
        : fullValue.slice(0, position.character - range.start.character);
    const workspaceRoot = resolveWorkspaceRoot(document, workspaceFolders);
    const currentDocFsPath = vscode_uri_1.default.parse(document.uri).fsPath;
    const paths = providePaths(valueBeforeCursor, currentDocFsPath, workspaceRoot)
        .filter(p => {
        // Exclude current doc's path
        return path.resolve(currentDocFsPath, '../', p) !== currentDocFsPath;
    })
        .filter(p => {
        // Exclude paths that start with `.`
        return p[0] !== '.';
    });
    const fullValueRange = isValueQuoted ? shiftRange(range, 1, -1) : range;
    const replaceRange = pathToReplaceRange(valueBeforeCursor, fullValue, fullValueRange);
    const suggestions = paths.map(p => pathToSuggestion(p, replaceRange));
    return suggestions;
}
function shouldDoPathCompletion(pathValue, workspaceFolders) {
    const fullValue = stripQuotes(pathValue);
    if (fullValue === '.' || fullValue === '..') {
        return false;
    }
    if (!workspaceFolders || workspaceFolders.length === 0) {
        return false;
    }
    return true;
}
function stripQuotes(fullValue) {
    if (strings_1.startsWith(fullValue, `'`) || strings_1.startsWith(fullValue, `"`)) {
        return fullValue.slice(1, -1);
    }
    else {
        return fullValue;
    }
}
/**
 * Get a list of path suggestions. Folder suggestions are suffixed with a slash.
 */
function providePaths(valueBeforeCursor, activeDocFsPath, root) {
    const lastIndexOfSlash = valueBeforeCursor.lastIndexOf('/');
    const valueBeforeLastSlash = valueBeforeCursor.slice(0, lastIndexOfSlash + 1);
    const startsWithSlash = strings_1.startsWith(valueBeforeCursor, '/');
    let parentDir;
    if (startsWithSlash) {
        if (!root) {
            return [];
        }
        parentDir = path.resolve(root, '.' + valueBeforeLastSlash);
    }
    else {
        parentDir = path.resolve(activeDocFsPath, '..', valueBeforeLastSlash);
    }
    try {
        return fs.readdirSync(parentDir).map(f => {
            return isDir(path.resolve(parentDir, f))
                ? f + '/'
                : f;
        });
    }
    catch (e) {
        return [];
    }
}
const isDir = (p) => {
    try {
        return fs.statSync(p).isDirectory();
    }
    catch (e) {
        return false;
    }
};
function pathToReplaceRange(valueBeforeCursor, fullValue, fullValueRange) {
    let replaceRange;
    const lastIndexOfSlash = valueBeforeCursor.lastIndexOf('/');
    if (lastIndexOfSlash === -1) {
        replaceRange = fullValueRange;
    }
    else {
        // For cases where cursor is in the middle of attribute value, like <script src="./s|rc/test.js">
        // Find the last slash before cursor, and calculate the start of replace range from there
        const valueAfterLastSlash = fullValue.slice(lastIndexOfSlash + 1);
        const startPos = shiftPosition(fullValueRange.end, -valueAfterLastSlash.length);
        // If whitespace exists, replace until it
        const whiteSpaceIndex = valueAfterLastSlash.indexOf(' ');
        let endPos;
        if (whiteSpaceIndex !== -1) {
            endPos = shiftPosition(startPos, whiteSpaceIndex);
        }
        else {
            endPos = fullValueRange.end;
        }
        replaceRange = vscode_languageserver_types_1.Range.create(startPos, endPos);
    }
    return replaceRange;
}
function pathToSuggestion(p, replaceRange) {
    const isDir = p[p.length - 1] === '/';
    if (isDir) {
        return {
            label: escapePath(p),
            kind: vscode_languageserver_types_1.CompletionItemKind.Folder,
            textEdit: vscode_languageserver_types_1.TextEdit.replace(replaceRange, escapePath(p)),
            command: {
                title: 'Suggest',
                command: 'editor.action.triggerSuggest'
            }
        };
    }
    else {
        return {
            label: escapePath(p),
            kind: vscode_languageserver_types_1.CompletionItemKind.File,
            textEdit: vscode_languageserver_types_1.TextEdit.replace(replaceRange, escapePath(p))
        };
    }
}
// Escape https://www.w3.org/TR/CSS1/#url
function escapePath(p) {
    return p.replace(/(\s|\(|\)|,|"|')/g, '\\$1');
}
function resolveWorkspaceRoot(activeDoc, workspaceFolders) {
    for (const folder of workspaceFolders) {
        if (strings_1.startsWith(activeDoc.uri, folder.uri)) {
            return path.resolve(vscode_uri_1.default.parse(folder.uri).fsPath);
        }
    }
    return undefined;
}
function shiftPosition(pos, offset) {
    return vscode_languageserver_types_1.Position.create(pos.line, pos.character + offset);
}
function shiftRange(range, startOffset, endOffset) {
    const start = shiftPosition(range.start, startOffset);
    const end = shiftPosition(range.end, endOffset);
    return vscode_languageserver_types_1.Range.create(start, end);
}
//# sourceMappingURL=https://ticino.blob.core.windows.net/sourcemaps/0e4914e21e8fd9d3eb22a61780ef74692ab25bdc/extensions/css-language-features/server/out/pathCompletion.js.map
