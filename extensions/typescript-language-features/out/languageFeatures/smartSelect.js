"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.register = void 0;
const vscode = require("vscode");
const api_1 = require("../utils/api");
const dependentRegistration_1 = require("../utils/dependentRegistration");
const typeConverters = require("../utils/typeConverters");
class SmartSelection {
    constructor(client) {
        this.client = client;
    }
    async provideSelectionRanges(document, positions, token) {
        const file = this.client.toOpenedFilePath(document);
        if (!file) {
            return undefined;
        }
        const args = {
            file,
            locations: positions.map(typeConverters.Position.toLocation)
        };
        const response = await this.client.execute('selectionRange', args, token);
        if (response.type !== 'response' || !response.body) {
            return undefined;
        }
        return response.body.map(SmartSelection.convertSelectionRange);
    }
    static convertSelectionRange(selectionRange) {
        return new vscode.SelectionRange(typeConverters.Range.fromTextSpan(selectionRange.textSpan), selectionRange.parent ? SmartSelection.convertSelectionRange(selectionRange.parent) : undefined);
    }
}
SmartSelection.minVersion = api_1.default.v350;
function register(selector, client) {
    return dependentRegistration_1.conditionalRegistration([
        dependentRegistration_1.requireMinVersion(client, SmartSelection.minVersion),
    ], () => {
        return vscode.languages.registerSelectionRangeProvider(selector.syntax, new SmartSelection(client));
    });
}
exports.register = register;
//# sourceMappingURL=smartSelect.js.map