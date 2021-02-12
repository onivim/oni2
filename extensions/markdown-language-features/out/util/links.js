"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.MarkdownFileExtensions = exports.isOfScheme = exports.getUriForLinkWithKnownExternalScheme = exports.Schemes = void 0;
const vscode = require("vscode");
exports.Schemes = {
    http: 'http:',
    https: 'https:',
    file: 'file:',
    untitled: 'untitled',
    mailto: 'mailto:',
    data: 'data:',
    vscode: 'vscode:',
    'vscode-insiders': 'vscode-insiders:',
    'vscode-resource': 'vscode-resource:',
};
const knownSchemes = [
    ...Object.values(exports.Schemes),
    `${vscode.env.uriScheme}:`
];
function getUriForLinkWithKnownExternalScheme(link) {
    if (knownSchemes.some(knownScheme => isOfScheme(knownScheme, link))) {
        return vscode.Uri.parse(link);
    }
    return undefined;
}
exports.getUriForLinkWithKnownExternalScheme = getUriForLinkWithKnownExternalScheme;
function isOfScheme(scheme, link) {
    return link.toLowerCase().startsWith(scheme);
}
exports.isOfScheme = isOfScheme;
exports.MarkdownFileExtensions = [
    '.md',
    '.mkd',
    '.mdwn',
    '.mdown',
    '.markdown',
    '.markdn',
    '.mdtxt',
    '.mdtext',
    '.workbook',
];
//# sourceMappingURL=links.js.map