"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const temp = require("./temp");
const path = require("path");
const fs = require("fs");
const cp = require("child_process");
const getRootTempDir = (() => {
    let dir;
    return () => {
        if (!dir) {
            dir = temp.getTempFile(`vscode-typescript`);
        }
        if (!fs.existsSync(dir)) {
            fs.mkdirSync(dir);
        }
        return dir;
    };
})();
function getTempFile(prefix) {
    return path.join(getRootTempDir(), `${prefix}-${temp.makeRandomHexString(20)}.tmp`);
}
exports.getTempFile = getTempFile;
function generatePatchedEnv(env, modulePath) {
    const newEnv = Object.assign({}, env);
    newEnv['ELECTRON_RUN_AS_NODE'] = '1';
    newEnv['NODE_PATH'] = path.join(modulePath, '..', '..', '..');
    // Ensure we always have a PATH set
    newEnv['PATH'] = newEnv['PATH'] || process.env.PATH;
    return newEnv;
}
function fork(modulePath, args, options) {
    const newEnv = generatePatchedEnv(process.env, modulePath);
    return cp.fork(modulePath, args, {
        silent: true,
        cwd: options.cwd,
        env: newEnv,
        execArgv: options.execArgv
    });
}
exports.fork = fork;
//# sourceMappingURL=electron.js.map