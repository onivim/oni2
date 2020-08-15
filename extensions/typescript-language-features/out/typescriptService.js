"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.ServerResponse = void 0;
var ServerResponse;
(function (ServerResponse) {
    class Cancelled {
        constructor(reason) {
            this.reason = reason;
            this.type = 'cancelled';
        }
    }
    ServerResponse.Cancelled = Cancelled;
    ServerResponse.NoContent = { type: 'noContent' };
})(ServerResponse = exports.ServerResponse || (exports.ServerResponse = {}));
//# sourceMappingURL=typescriptService.js.map