"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.normalizeResource = void 0;
const vscode = require("vscode");
function normalizeResource(base, resource) {
    // If we  have a windows path and are loading a workspace with an authority,
    // make sure we use a unc path with an explicit localhost authority.
    //
    // Otherwise, the `<base>` rule will insert the authority into the resolved resource
    // URI incorrectly.
    if (base.authority && !resource.authority) {
        const driveMatch = resource.path.match(/^\/(\w):\//);
        if (driveMatch) {
            return vscode.Uri.file(`\\\\localhost\\${driveMatch[1]}$\\${resource.fsPath.replace(/^\w:\\/, '')}`).with({
                fragment: resource.fragment,
                query: resource.query
            });
        }
    }
    return resource;
}
exports.normalizeResource = normalizeResource;
//# sourceMappingURL=resources.js.map