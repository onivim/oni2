"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.Log = exports.LogLevel = void 0;
const vscode_1 = require("vscode");
/**
 * The severity level of a log message
 */
var LogLevel;
(function (LogLevel) {
    LogLevel[LogLevel["Trace"] = 1] = "Trace";
    LogLevel[LogLevel["Debug"] = 2] = "Debug";
    LogLevel[LogLevel["Info"] = 3] = "Info";
    LogLevel[LogLevel["Warning"] = 4] = "Warning";
    LogLevel[LogLevel["Error"] = 5] = "Error";
    LogLevel[LogLevel["Critical"] = 6] = "Critical";
    LogLevel[LogLevel["Off"] = 7] = "Off";
})(LogLevel = exports.LogLevel || (exports.LogLevel = {}));
let _logLevel = LogLevel.Info;
const _onDidChangeLogLevel = new vscode_1.EventEmitter();
exports.Log = {
    /**
     * Current logging level.
     */
    get logLevel() {
        return _logLevel;
    },
    /**
     * Current logging level.
     */
    set logLevel(logLevel) {
        if (_logLevel === logLevel) {
            return;
        }
        _logLevel = logLevel;
        _onDidChangeLogLevel.fire(logLevel);
    },
    /**
     * An [event](#Event) that fires when the log level has changed.
     */
    get onDidChangeLogLevel() {
        return _onDidChangeLogLevel.event;
    }
};
//# sourceMappingURL=log.js.map