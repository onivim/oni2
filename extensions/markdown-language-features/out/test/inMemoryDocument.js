"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.InMemoryDocument = void 0;
const vscode = require("vscode");
const os = require("os");
class InMemoryDocument {
    constructor(uri, _contents, version = 1) {
        this.uri = uri;
        this._contents = _contents;
        this.version = version;
        this.isUntitled = false;
        this.languageId = '';
        this.isDirty = false;
        this.isClosed = false;
        this.eol = os.platform() === 'win32' ? vscode.EndOfLine.CRLF : vscode.EndOfLine.LF;
        this._lines = this._contents.split(/\r\n|\n/g);
    }
    get fileName() {
        return this.uri.fsPath;
    }
    get lineCount() {
        return this._lines.length;
    }
    lineAt(line) {
        return {
            lineNumber: line,
            text: this._lines[line],
            range: new vscode.Range(0, 0, 0, 0),
            firstNonWhitespaceCharacterIndex: 0,
            rangeIncludingLineBreak: new vscode.Range(0, 0, 0, 0),
            isEmptyOrWhitespace: false
        };
    }
    offsetAt(_position) {
        throw new Error('Method not implemented.');
    }
    positionAt(offset) {
        const before = this._contents.slice(0, offset);
        const newLines = before.match(/\r\n|\n/g);
        const line = newLines ? newLines.length : 0;
        const preCharacters = before.match(/(\r\n|\n|^).*$/g);
        return new vscode.Position(line, preCharacters ? preCharacters[0].length : 0);
    }
    getText(_range) {
        return this._contents;
    }
    getWordRangeAtPosition(_position, _regex) {
        throw new Error('Method not implemented.');
    }
    validateRange(_range) {
        throw new Error('Method not implemented.');
    }
    validatePosition(_position) {
        throw new Error('Method not implemented.');
    }
    save() {
        throw new Error('Method not implemented.');
    }
}
exports.InMemoryDocument = InMemoryDocument;
//# sourceMappingURL=inMemoryDocument.js.map