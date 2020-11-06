"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.watch = void 0;
const vscode_1 = require("vscode");
const path_1 = require("path");
const fs = require("fs");
function watch(location) {
    const dotGitWatcher = fs.watch(location);
    const onDotGitFileChangeEmitter = new vscode_1.EventEmitter();
    dotGitWatcher.on('change', (_, e) => onDotGitFileChangeEmitter.fire(vscode_1.Uri.file(path_1.join(location, e))));
    dotGitWatcher.on('error', err => console.error(err));
    return new class {
        constructor() {
            this.event = onDotGitFileChangeEmitter.event;
        }
        dispose() { dotGitWatcher.close(); }
    };
}
exports.watch = watch;
//# sourceMappingURL=watch.js.map