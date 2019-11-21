"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const vscode_languageserver_1 = require("vscode-languageserver");
function formatError(message, err) {
    if (err instanceof Error) {
        let error = err;
        return `${message}: ${error.message}\n${error.stack}`;
    }
    else if (typeof err === 'string') {
        return `${message}: ${err}`;
    }
    else if (err) {
        return `${message}: ${err.toString()}`;
    }
    return message;
}
exports.formatError = formatError;
function runSafe(func, errorVal, errorMessage, token) {
    return new Promise((resolve) => {
        setImmediate(() => {
            if (token.isCancellationRequested) {
                resolve(cancelValue());
            }
            else {
                try {
                    let result = func();
                    if (token.isCancellationRequested) {
                        resolve(cancelValue());
                        return;
                    }
                    else {
                        resolve(result);
                    }
                }
                catch (e) {
                    console.error(formatError(errorMessage, e));
                    resolve(errorVal);
                }
            }
        });
    });
}
exports.runSafe = runSafe;
function cancelValue() {
    return new vscode_languageserver_1.ResponseError(vscode_languageserver_1.ErrorCodes.RequestCancelled, 'Request cancelled');
}
//# sourceMappingURL=runner.js.map