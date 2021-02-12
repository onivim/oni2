"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.getDocumentRegions = exports.CSS_STYLE_RULE = void 0;
const languageModes_1 = require("./languageModes");
exports.CSS_STYLE_RULE = '__';
function getDocumentRegions(languageService, document) {
    let regions = [];
    let scanner = languageService.createScanner(document.getText());
    let lastTagName = '';
    let lastAttributeName = null;
    let languageIdFromType = undefined;
    let importedScripts = [];
    let token = scanner.scan();
    while (token !== languageModes_1.TokenType.EOS) {
        switch (token) {
            case languageModes_1.TokenType.StartTag:
                lastTagName = scanner.getTokenText();
                lastAttributeName = null;
                languageIdFromType = 'javascript';
                break;
            case languageModes_1.TokenType.Styles:
                regions.push({ languageId: 'css', start: scanner.getTokenOffset(), end: scanner.getTokenEnd() });
                break;
            case languageModes_1.TokenType.Script:
                regions.push({ languageId: languageIdFromType, start: scanner.getTokenOffset(), end: scanner.getTokenEnd() });
                break;
            case languageModes_1.TokenType.AttributeName:
                lastAttributeName = scanner.getTokenText();
                break;
            case languageModes_1.TokenType.AttributeValue:
                if (lastAttributeName === 'src' && lastTagName.toLowerCase() === 'script') {
                    let value = scanner.getTokenText();
                    if (value[0] === '\'' || value[0] === '"') {
                        value = value.substr(1, value.length - 1);
                    }
                    importedScripts.push(value);
                }
                else if (lastAttributeName === 'type' && lastTagName.toLowerCase() === 'script') {
                    if (/["'](module|(text|application)\/(java|ecma)script|text\/babel)["']/.test(scanner.getTokenText())) {
                        languageIdFromType = 'javascript';
                    }
                    else if (/["']text\/typescript["']/.test(scanner.getTokenText())) {
                        languageIdFromType = 'typescript';
                    }
                    else {
                        languageIdFromType = undefined;
                    }
                }
                else {
                    let attributeLanguageId = getAttributeLanguage(lastAttributeName);
                    if (attributeLanguageId) {
                        let start = scanner.getTokenOffset();
                        let end = scanner.getTokenEnd();
                        let firstChar = document.getText()[start];
                        if (firstChar === '\'' || firstChar === '"') {
                            start++;
                            end--;
                        }
                        regions.push({ languageId: attributeLanguageId, start, end, attributeValue: true });
                    }
                }
                lastAttributeName = null;
                break;
        }
        token = scanner.scan();
    }
    return {
        getLanguageRanges: (range) => getLanguageRanges(document, regions, range),
        getEmbeddedDocument: (languageId, ignoreAttributeValues) => getEmbeddedDocument(document, regions, languageId, ignoreAttributeValues),
        getLanguageAtPosition: (position) => getLanguageAtPosition(document, regions, position),
        getLanguagesInDocument: () => getLanguagesInDocument(document, regions),
        getImportedScripts: () => importedScripts
    };
}
exports.getDocumentRegions = getDocumentRegions;
function getLanguageRanges(document, regions, range) {
    let result = [];
    let currentPos = range ? range.start : languageModes_1.Position.create(0, 0);
    let currentOffset = range ? document.offsetAt(range.start) : 0;
    let endOffset = range ? document.offsetAt(range.end) : document.getText().length;
    for (let region of regions) {
        if (region.end > currentOffset && region.start < endOffset) {
            let start = Math.max(region.start, currentOffset);
            let startPos = document.positionAt(start);
            if (currentOffset < region.start) {
                result.push({
                    start: currentPos,
                    end: startPos,
                    languageId: 'html'
                });
            }
            let end = Math.min(region.end, endOffset);
            let endPos = document.positionAt(end);
            if (end > region.start) {
                result.push({
                    start: startPos,
                    end: endPos,
                    languageId: region.languageId,
                    attributeValue: region.attributeValue
                });
            }
            currentOffset = end;
            currentPos = endPos;
        }
    }
    if (currentOffset < endOffset) {
        let endPos = range ? range.end : document.positionAt(endOffset);
        result.push({
            start: currentPos,
            end: endPos,
            languageId: 'html'
        });
    }
    return result;
}
function getLanguagesInDocument(_document, regions) {
    let result = [];
    for (let region of regions) {
        if (region.languageId && result.indexOf(region.languageId) === -1) {
            result.push(region.languageId);
            if (result.length === 3) {
                return result;
            }
        }
    }
    result.push('html');
    return result;
}
function getLanguageAtPosition(document, regions, position) {
    let offset = document.offsetAt(position);
    for (let region of regions) {
        if (region.start <= offset) {
            if (offset <= region.end) {
                return region.languageId;
            }
        }
        else {
            break;
        }
    }
    return 'html';
}
function getEmbeddedDocument(document, contents, languageId, ignoreAttributeValues) {
    let currentPos = 0;
    let oldContent = document.getText();
    let result = '';
    let lastSuffix = '';
    for (let c of contents) {
        if (c.languageId === languageId && (!ignoreAttributeValues || !c.attributeValue)) {
            result = substituteWithWhitespace(result, currentPos, c.start, oldContent, lastSuffix, getPrefix(c));
            result += oldContent.substring(c.start, c.end);
            currentPos = c.end;
            lastSuffix = getSuffix(c);
        }
    }
    result = substituteWithWhitespace(result, currentPos, oldContent.length, oldContent, lastSuffix, '');
    return languageModes_1.TextDocument.create(document.uri, languageId, document.version, result);
}
function getPrefix(c) {
    if (c.attributeValue) {
        switch (c.languageId) {
            case 'css': return exports.CSS_STYLE_RULE + '{';
        }
    }
    return '';
}
function getSuffix(c) {
    if (c.attributeValue) {
        switch (c.languageId) {
            case 'css': return '}';
            case 'javascript': return ';';
        }
    }
    return '';
}
function substituteWithWhitespace(result, start, end, oldContent, before, after) {
    let accumulatedWS = 0;
    result += before;
    for (let i = start + before.length; i < end; i++) {
        let ch = oldContent[i];
        if (ch === '\n' || ch === '\r') {
            // only write new lines, skip the whitespace
            accumulatedWS = 0;
            result += ch;
        }
        else {
            accumulatedWS++;
        }
    }
    result = append(result, ' ', accumulatedWS - after.length);
    result += after;
    return result;
}
function append(result, str, n) {
    while (n > 0) {
        if (n & 1) {
            result += str;
        }
        n >>= 1;
        str += str;
    }
    return result;
}
function getAttributeLanguage(attributeName) {
    let match = attributeName.match(/^(style)$|^(on\w+)$/i);
    if (!match) {
        return null;
    }
    return match[1] ? 'css' : 'javascript';
}
//# sourceMappingURL=embeddedSupport.js.map