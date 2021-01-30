"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.isNewlineCharacter = exports.isEOL = exports.isWhitespaceOnly = exports.repeat = exports.endsWith = exports.startsWith = exports.getWordAtText = void 0;
function getWordAtText(text, offset, wordDefinition) {
    let lineStart = offset;
    while (lineStart > 0 && !isNewlineCharacter(text.charCodeAt(lineStart - 1))) {
        lineStart--;
    }
    let offsetInLine = offset - lineStart;
    let lineText = text.substr(lineStart);
    // make a copy of the regex as to not keep the state
    let flags = wordDefinition.ignoreCase ? 'gi' : 'g';
    wordDefinition = new RegExp(wordDefinition.source, flags);
    let match = wordDefinition.exec(lineText);
    while (match && match.index + match[0].length < offsetInLine) {
        match = wordDefinition.exec(lineText);
    }
    if (match && match.index <= offsetInLine) {
        return { start: match.index + lineStart, length: match[0].length };
    }
    return { start: offset, length: 0 };
}
exports.getWordAtText = getWordAtText;
function startsWith(haystack, needle) {
    if (haystack.length < needle.length) {
        return false;
    }
    for (let i = 0; i < needle.length; i++) {
        if (haystack[i] !== needle[i]) {
            return false;
        }
    }
    return true;
}
exports.startsWith = startsWith;
function endsWith(haystack, needle) {
    let diff = haystack.length - needle.length;
    if (diff > 0) {
        return haystack.indexOf(needle, diff) === diff;
    }
    else if (diff === 0) {
        return haystack === needle;
    }
    else {
        return false;
    }
}
exports.endsWith = endsWith;
function repeat(value, count) {
    let s = '';
    while (count > 0) {
        if ((count & 1) === 1) {
            s += value;
        }
        value += value;
        count = count >>> 1;
    }
    return s;
}
exports.repeat = repeat;
function isWhitespaceOnly(str) {
    return /^\s*$/.test(str);
}
exports.isWhitespaceOnly = isWhitespaceOnly;
function isEOL(content, offset) {
    return isNewlineCharacter(content.charCodeAt(offset));
}
exports.isEOL = isEOL;
const CR = '\r'.charCodeAt(0);
const NL = '\n'.charCodeAt(0);
function isNewlineCharacter(charCode) {
    return charCode === CR || charCode === NL;
}
exports.isNewlineCharacter = isNewlineCharacter;
//# sourceMappingURL=strings.js.map