"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.joinLines = void 0;
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
const os = require("os");
const joinLines = (...args) => args.join(os.platform() === 'win32' ? '\r\n' : '\n');
exports.joinLines = joinLines;
//# sourceMappingURL=util.js.map