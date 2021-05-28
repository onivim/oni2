"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.updateImageSize = void 0;
// Based on @sergeche's work on the emmet plugin for atom
const vscode_1 = require("vscode");
const path = require("path");
const imageSizeHelper_1 = require("./imageSizeHelper");
const util_1 = require("./util");
const locateFile_1 = require("./locateFile");
const css_parser_1 = require("@emmetio/css-parser");
const bufferStream_1 = require("./bufferStream");
/**
 * Updates size of context image in given editor
 */
function updateImageSize() {
    if (!util_1.validate() || !vscode_1.window.activeTextEditor) {
        return;
    }
    const editor = vscode_1.window.activeTextEditor;
    let allUpdatesPromise = editor.selections.reverse().map(selection => {
        let position = selection.isReversed ? selection.active : selection.anchor;
        if (!util_1.isStyleSheet(editor.document.languageId)) {
            return updateImageSizeHTML(editor, position);
        }
        else {
            return updateImageSizeCSSFile(editor, position);
        }
    });
    return Promise.all(allUpdatesPromise).then((updates) => {
        return editor.edit(builder => {
            updates.forEach(update => {
                update.forEach((textEdit) => {
                    builder.replace(textEdit.range, textEdit.newText);
                });
            });
        });
    });
}
exports.updateImageSize = updateImageSize;
/**
 * Updates image size of context tag of HTML model
 */
function updateImageSizeHTML(editor, position) {
    const imageNode = getImageHTMLNode(editor, position);
    const src = imageNode && getImageSrcHTML(imageNode);
    if (!src) {
        return updateImageSizeStyleTag(editor, position);
    }
    return locateFile_1.locateFile(path.dirname(editor.document.fileName), src)
        .then(imageSizeHelper_1.getImageSize)
        .then((size) => {
        // since this action is asynchronous, we have to ensure that editor wasn’t
        // changed and user didn’t moved caret outside <img> node
        const img = getImageHTMLNode(editor, position);
        if (img && getImageSrcHTML(img) === src) {
            return updateHTMLTag(editor, img, size.width, size.height);
        }
        return [];
    })
        .catch(err => { console.warn('Error while updating image size:', err); return []; });
}
function updateImageSizeStyleTag(editor, position) {
    const getPropertyInsiderStyleTag = (editor) => {
        const rootNode = util_1.parseDocument(editor.document);
        const currentNode = util_1.getNode(rootNode, position, true);
        if (currentNode && currentNode.name === 'style'
            && currentNode.open.end.isBefore(position)
            && currentNode.close.start.isAfter(position)) {
            let buffer = new bufferStream_1.DocumentStreamReader(editor.document, currentNode.open.end, new vscode_1.Range(currentNode.open.end, currentNode.close.start));
            let rootNode = css_parser_1.default(buffer);
            const node = util_1.getNode(rootNode, position, true);
            return (node && node.type === 'property') ? node : null;
        }
        return null;
    };
    return updateImageSizeCSS(editor, position, getPropertyInsiderStyleTag);
}
function updateImageSizeCSSFile(editor, position) {
    return updateImageSizeCSS(editor, position, getImageCSSNode);
}
/**
 * Updates image size of context rule of stylesheet model
 */
function updateImageSizeCSS(editor, position, fetchNode) {
    const node = fetchNode(editor, position);
    const src = node && getImageSrcCSS(node, position);
    if (!src) {
        return Promise.reject(new Error('No valid image source'));
    }
    return locateFile_1.locateFile(path.dirname(editor.document.fileName), src)
        .then(imageSizeHelper_1.getImageSize)
        .then((size) => {
        // since this action is asynchronous, we have to ensure that editor wasn’t
        // changed and user didn’t moved caret outside <img> node
        const prop = fetchNode(editor, position);
        if (prop && getImageSrcCSS(prop, position) === src) {
            return updateCSSNode(editor, prop, size.width, size.height);
        }
        return [];
    })
        .catch(err => { console.warn('Error while updating image size:', err); return []; });
}
/**
 * Returns <img> node under caret in given editor or `null` if such node cannot
 * be found
 */
function getImageHTMLNode(editor, position) {
    const rootNode = util_1.parseDocument(editor.document);
    const node = util_1.getNode(rootNode, position, true);
    return node && node.name.toLowerCase() === 'img' ? node : null;
}
/**
 * Returns css property under caret in given editor or `null` if such node cannot
 * be found
 */
function getImageCSSNode(editor, position) {
    const rootNode = util_1.parseDocument(editor.document);
    const node = util_1.getNode(rootNode, position, true);
    return node && node.type === 'property' ? node : null;
}
/**
 * Returns image source from given <img> node
 */
function getImageSrcHTML(node) {
    const srcAttr = getAttribute(node, 'src');
    if (!srcAttr) {
        return;
    }
    return srcAttr.value.value;
}
/**
 * Returns image source from given `url()` token
 */
function getImageSrcCSS(node, position) {
    if (!node) {
        return;
    }
    const urlToken = findUrlToken(node, position);
    if (!urlToken) {
        return;
    }
    // A stylesheet token may contain either quoted ('string') or unquoted URL
    let urlValue = urlToken.item(0);
    if (urlValue && urlValue.type === 'string') {
        urlValue = urlValue.item(0);
    }
    return urlValue && urlValue.valueOf();
}
/**
 * Updates size of given HTML node
 */
function updateHTMLTag(editor, node, width, height) {
    const srcAttr = getAttribute(node, 'src');
    const widthAttr = getAttribute(node, 'width');
    const heightAttr = getAttribute(node, 'height');
    const quote = getAttributeQuote(editor, srcAttr);
    const endOfAttributes = node.attributes[node.attributes.length - 1].end;
    let edits = [];
    let textToAdd = '';
    if (!widthAttr) {
        textToAdd += ` width=${quote}${width}${quote}`;
    }
    else {
        edits.push(new vscode_1.TextEdit(new vscode_1.Range(widthAttr.value.start, widthAttr.value.end), String(width)));
    }
    if (!heightAttr) {
        textToAdd += ` height=${quote}${height}${quote}`;
    }
    else {
        edits.push(new vscode_1.TextEdit(new vscode_1.Range(heightAttr.value.start, heightAttr.value.end), String(height)));
    }
    if (textToAdd) {
        edits.push(new vscode_1.TextEdit(new vscode_1.Range(endOfAttributes, endOfAttributes), textToAdd));
    }
    return edits;
}
/**
 * Updates size of given CSS rule
 */
function updateCSSNode(editor, srcProp, width, height) {
    const rule = srcProp.parent;
    const widthProp = util_1.getCssPropertyFromRule(rule, 'width');
    const heightProp = util_1.getCssPropertyFromRule(rule, 'height');
    // Detect formatting
    const separator = srcProp.separator || ': ';
    const before = getPropertyDelimitor(editor, srcProp);
    let edits = [];
    if (!srcProp.terminatorToken) {
        edits.push(new vscode_1.TextEdit(new vscode_1.Range(srcProp.end, srcProp.end), ';'));
    }
    let textToAdd = '';
    if (!widthProp) {
        textToAdd += `${before}width${separator}${width}px;`;
    }
    else {
        edits.push(new vscode_1.TextEdit(new vscode_1.Range(widthProp.valueToken.start, widthProp.valueToken.end), `${width}px`));
    }
    if (!heightProp) {
        textToAdd += `${before}height${separator}${height}px;`;
    }
    else {
        edits.push(new vscode_1.TextEdit(new vscode_1.Range(heightProp.valueToken.start, heightProp.valueToken.end), `${height}px`));
    }
    if (textToAdd) {
        edits.push(new vscode_1.TextEdit(new vscode_1.Range(srcProp.end, srcProp.end), textToAdd));
    }
    return edits;
}
/**
 * Returns attribute object with `attrName` name from given HTML node
 */
function getAttribute(node, attrName) {
    attrName = attrName.toLowerCase();
    return node && node.open.attributes.find((attr) => attr.name.value.toLowerCase() === attrName);
}
/**
 * Returns quote character, used for value of given attribute. May return empty
 * string if attribute wasn’t quoted

 */
function getAttributeQuote(editor, attr) {
    const range = new vscode_1.Range(attr.value ? attr.value.end : attr.end, attr.end);
    return range.isEmpty ? '' : editor.document.getText(range);
}
/**
 * Finds 'url' token for given `pos` point in given CSS property `node`
 */
function findUrlToken(node, pos) {
    for (let i = 0, il = node.parsedValue.length, url; i < il; i++) {
        util_1.iterateCSSToken(node.parsedValue[i], (token) => {
            if (token.type === 'url' && token.start.isBeforeOrEqual(pos) && token.end.isAfterOrEqual(pos)) {
                url = token;
                return false;
            }
            return true;
        });
        if (url) {
            return url;
        }
    }
    return;
}
/**
 * Returns a string that is used to delimit properties in current node’s rule
 */
function getPropertyDelimitor(editor, node) {
    let anchor;
    if (anchor = (node.previousSibling || node.parent.contentStartToken)) {
        return editor.document.getText(new vscode_1.Range(anchor.end, node.start));
    }
    else if (anchor = (node.nextSibling || node.parent.contentEndToken)) {
        return editor.document.getText(new vscode_1.Range(node.end, anchor.start));
    }
    return '';
}
//# sourceMappingURL=updateImageSize.js.map