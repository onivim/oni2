"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.DocumentStreamReader = void 0;
/* Based on @sergeche's work in his emmet plugin */
const vscode_1 = require("vscode");
/**
 * A stream reader for VSCode's `TextDocument`
 * Based on @emmetio/stream-reader and @emmetio/atom-plugin
 */
class DocumentStreamReader {
    constructor(document, pos, limit) {
        this.document = document;
        this.start = this.pos = pos ? pos : new vscode_1.Position(0, 0);
        this._sof = limit ? limit.start : new vscode_1.Position(0, 0);
        this._eof = limit ? limit.end : new vscode_1.Position(this.document.lineCount - 1, this._lineLength(this.document.lineCount - 1));
        this._eol = this.document.eol === vscode_1.EndOfLine.LF ? '\n' : '\r\n';
    }
    /**
     * Returns true only if the stream is at the start of the file.
     */
    sof() {
        return this.pos.isBeforeOrEqual(this._sof);
    }
    /**
     * Returns true only if the stream is at the end of the file.
     */
    eof() {
        return this.pos.isAfterOrEqual(this._eof);
    }
    /**
     * Creates a new stream instance which is limited to given range for given document
     */
    limit(start, end) {
        return new DocumentStreamReader(this.document, start, new vscode_1.Range(start, end));
    }
    /**
     * Returns the next character code in the stream without advancing it.
     * Will return NaN at the end of the file.
     */
    peek() {
        if (this.eof()) {
            return NaN;
        }
        const line = this.document.lineAt(this.pos.line).text;
        return this.pos.character < line.length ? line.charCodeAt(this.pos.character) : this._eol.charCodeAt(this.pos.character - line.length);
    }
    /**
     * Returns the next character in the stream and advances it.
     * Also returns NaN when no more characters are available.
     */
    next() {
        if (this.eof()) {
            return NaN;
        }
        const line = this.document.lineAt(this.pos.line).text;
        let code;
        if (this.pos.character < line.length) {
            code = line.charCodeAt(this.pos.character);
            this.pos = this.pos.translate(0, 1);
        }
        else {
            code = this._eol.charCodeAt(this.pos.character - line.length);
            this.pos = new vscode_1.Position(this.pos.line + 1, 0);
        }
        if (this.eof()) {
            // restrict pos to eof, if in case it got moved beyond eof
            this.pos = new vscode_1.Position(this._eof.line, this._eof.character);
        }
        return code;
    }
    /**
     * Backs up the stream n characters. Backing it up further than the
     * start of the current token will cause things to break, so be careful.
     */
    backUp(n) {
        let row = this.pos.line;
        let column = this.pos.character;
        column -= (n || 1);
        while (row >= 0 && column < 0) {
            row--;
            column += this._lineLength(row);
        }
        this.pos = row < 0 || column < 0
            ? new vscode_1.Position(0, 0)
            : new vscode_1.Position(row, column);
        return this.peek();
    }
    /**
     * Get the string between the start of the current token and the
     * current stream position.
     */
    current() {
        return this.substring(this.start, this.pos);
    }
    /**
     * Returns contents for given range
     */
    substring(from, to) {
        return this.document.getText(new vscode_1.Range(from, to));
    }
    /**
     * Creates error object with current stream state
     */
    error(message) {
        const err = new Error(`${message} at row ${this.pos.line}, column ${this.pos.character}`);
        return err;
    }
    /**
     * Returns line length of given row, including line ending
     */
    _lineLength(row) {
        if (row === this.document.lineCount - 1) {
            return this.document.lineAt(row).text.length;
        }
        return this.document.lineAt(row).text.length + this._eol.length;
    }
    /**
     * `match` can be a character code or a function that takes a character code
     * and returns a boolean. If the next character in the stream 'matches'
     * the given argument, it is consumed and returned.
     * Otherwise, `false` is returned.
     */
    eat(match) {
        const ch = this.peek();
        const ok = typeof match === 'function' ? match(ch) : ch === match;
        if (ok) {
            this.next();
        }
        return ok;
    }
    /**
     * Repeatedly calls <code>eat</code> with the given argument, until it
     * fails. Returns <code>true</code> if any characters were eaten.
     */
    eatWhile(match) {
        const start = this.pos;
        while (!this.eof() && this.eat(match)) { }
        return !this.pos.isEqual(start);
    }
}
exports.DocumentStreamReader = DocumentStreamReader;
//# sourceMappingURL=bufferStream.js.map