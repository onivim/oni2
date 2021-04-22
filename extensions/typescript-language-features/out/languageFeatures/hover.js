"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.register = void 0;
const vscode = require("vscode");
const versionProvider_1 = require("../tsServer/versionProvider");
const typescriptService_1 = require("../typescriptService");
const dependentRegistration_1 = require("../utils/dependentRegistration");
const previewer_1 = require("../utils/previewer");
const typeConverters = require("../utils/typeConverters");
class TypeScriptHoverProvider {
    constructor(client) {
        this.client = client;
    }
    async provideHover(document, position, token) {
        const filepath = this.client.toOpenedFilePath(document);
        if (!filepath) {
            return undefined;
        }
        const args = typeConverters.Position.toFileLocationRequestArgs(filepath, position);
        const response = await this.client.interruptGetErr(() => this.client.execute('quickinfo', args, token));
        if (response.type !== 'response' || !response.body) {
            return undefined;
        }
        return new vscode.Hover(this.getContents(document.uri, response.body, response._serverType), typeConverters.Range.fromTextSpan(response.body));
    }
    getContents(resource, data, source) {
        const parts = [];
        if (data.displayString) {
            const displayParts = [];
            if (source === typescriptService_1.ServerType.Syntax && this.client.hasCapabilityForResource(resource, typescriptService_1.ClientCapability.Semantic)) {
                displayParts.push(versionProvider_1.localize({
                    key: 'loadingPrefix',
                    comment: ['Prefix displayed for hover entries while the server is still loading']
                }, "(loading...)"));
            }
            displayParts.push(data.displayString);
            parts.push({ language: 'typescript', value: displayParts.join(' ') });
        }
        parts.push(previewer_1.markdownDocumentation(data.documentation, data.tags));
        return parts;
    }
}
function register(selector, client) {
    return dependentRegistration_1.conditionalRegistration([
        dependentRegistration_1.requireSomeCapability(client, typescriptService_1.ClientCapability.EnhancedSyntax, typescriptService_1.ClientCapability.Semantic),
    ], () => {
        return vscode.languages.registerHoverProvider(selector.syntax, new TypeScriptHoverProvider(client));
    });
}
exports.register = register;
//# sourceMappingURL=hover.js.map