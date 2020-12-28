"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.deactivate = exports.activate = void 0;
const httpRequest = require("request-light");
const jsonContributions_1 = require("./features/jsonContributions");
async function activate(context) {
    context.subscriptions.push(jsonContributions_1.addJSONProviders(httpRequest.xhr, false));
}
exports.activate = activate;
function deactivate() {
}
exports.deactivate = deactivate;
//# sourceMappingURL=npmBrowserMain.js.map