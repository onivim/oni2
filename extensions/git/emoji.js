/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
'use strict';
Object.defineProperty(exports, "__esModule", { value: true });
exports.emojify = exports.ensureEmojis = void 0;
const vscode_1 = require("vscode");
const main_1 = require("./main");
const util_1 = require("util");
const emojiRegex = /:([-+_a-z0-9]+):/g;
let emojiMap;
let emojiMapPromise;
async function ensureEmojis() {
    if (emojiMap === undefined) {
        if (emojiMapPromise === undefined) {
            emojiMapPromise = loadEmojiMap();
        }
        await emojiMapPromise;
    }
}
exports.ensureEmojis = ensureEmojis;
async function loadEmojiMap() {
    const context = main_1.getExtensionContext();
    const uri = vscode_1.Uri.joinPath(context.extensionUri, 'resources', 'emojis.json');
    emojiMap = JSON.parse(new util_1.TextDecoder('utf8').decode(await vscode_1.workspace.fs.readFile(uri)));
}
function emojify(message) {
    if (emojiMap === undefined) {
        return message;
    }
    return message.replace(emojiRegex, (s, code) => {
        return (emojiMap === null || emojiMap === void 0 ? void 0 : emojiMap[code]) || s;
    });
}
exports.emojify = emojify;
//# sourceMappingURL=emoji.js.map