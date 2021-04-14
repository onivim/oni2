"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.reflectCssValue = void 0;
const vscode_1 = require("vscode");
const util_1 = require("./util");
const vendorPrefixes = ['-webkit-', '-moz-', '-ms-', '-o-', ''];
function reflectCssValue() {
    let editor = vscode_1.window.activeTextEditor;
    if (!editor) {
        vscode_1.window.showInformationMessage('No editor is active.');
        return;
    }
    let node = util_1.getCssPropertyFromDocument(editor, editor.selection.active);
    if (!node) {
        return;
    }
    return updateCSSNode(editor, node);
}
exports.reflectCssValue = reflectCssValue;
function updateCSSNode(editor, property) {
    const rule = property.parent;
    let currentPrefix = '';
    // Find vendor prefix of given property node
    for (const prefix of vendorPrefixes) {
        if (property.name.startsWith(prefix)) {
            currentPrefix = prefix;
            break;
        }
    }
    const propertyName = property.name.substr(currentPrefix.length);
    const propertyValue = property.value;
    return editor.edit(builder => {
        // Find properties with vendor prefixes, update each
        vendorPrefixes.forEach(prefix => {
            if (prefix === currentPrefix) {
                return;
            }
            let vendorProperty = util_1.getCssPropertyFromRule(rule, prefix + propertyName);
            if (vendorProperty) {
                builder.replace(new vscode_1.Range(vendorProperty.valueToken.start, vendorProperty.valueToken.end), propertyValue);
            }
        });
    });
}
//# sourceMappingURL=reflectCssValue.js.map