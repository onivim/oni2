"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.equalRange = exports.insideRangeButNotSame = exports.beforeOrSame = void 0;
function beforeOrSame(p1, p2) {
    return p1.line < p2.line || p1.line === p2.line && p1.character <= p2.character;
}
exports.beforeOrSame = beforeOrSame;
function insideRangeButNotSame(r1, r2) {
    return beforeOrSame(r1.start, r2.start) && beforeOrSame(r2.end, r1.end) && !equalRange(r1, r2);
}
exports.insideRangeButNotSame = insideRangeButNotSame;
function equalRange(r1, r2) {
    return r1.start.line === r2.start.line && r1.start.character === r2.start.character && r1.end.line === r2.end.line && r1.end.character === r2.end.character;
}
exports.equalRange = equalRange;
//# sourceMappingURL=positions.js.map