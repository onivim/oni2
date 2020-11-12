"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.register = void 0;
const vscode = require("vscode");
const typeConverters = require("../utils/typeConverters");
const api_1 = require("../utils/api");
const dependentRegistration_1 = require("../utils/dependentRegistration");
const path = require("path");
const PConst = require("../protocol.const");
let TypeScriptCallHierarchySupport = /** @class */ (() => {
    class TypeScriptCallHierarchySupport {
        constructor(client) {
            this.client = client;
        }
        async prepareCallHierarchy(document, position, token) {
            const filepath = this.client.toOpenedFilePath(document);
            if (!filepath) {
                return undefined;
            }
            const args = typeConverters.Position.toFileLocationRequestArgs(filepath, position);
            const response = await this.client.execute('prepareCallHierarchy', args, token);
            if (response.type !== 'response' || !response.body) {
                return undefined;
            }
            return Array.isArray(response.body)
                ? response.body.map(fromProtocolCallHierarchyItem)
                : fromProtocolCallHierarchyItem(response.body);
        }
        async provideCallHierarchyIncomingCalls(item, token) {
            const filepath = this.client.toPath(item.uri);
            if (!filepath) {
                return undefined;
            }
            const args = typeConverters.Position.toFileLocationRequestArgs(filepath, item.selectionRange.start);
            const response = await this.client.execute('provideCallHierarchyIncomingCalls', args, token);
            if (response.type !== 'response' || !response.body) {
                return undefined;
            }
            return response.body.map(fromProtocolCallHierchyIncomingCall);
        }
        async provideCallHierarchyOutgoingCalls(item, token) {
            const filepath = this.client.toPath(item.uri);
            if (!filepath) {
                return undefined;
            }
            const args = typeConverters.Position.toFileLocationRequestArgs(filepath, item.selectionRange.start);
            const response = await this.client.execute('provideCallHierarchyOutgoingCalls', args, token);
            if (response.type !== 'response' || !response.body) {
                return undefined;
            }
            return response.body.map(fromProtocolCallHierchyOutgoingCall);
        }
    }
    TypeScriptCallHierarchySupport.minVersion = api_1.default.v380;
    return TypeScriptCallHierarchySupport;
})();
function isSourceFileItem(item) {
    return item.kind === PConst.Kind.script || item.kind === PConst.Kind.module && item.selectionSpan.start.line === 1 && item.selectionSpan.start.offset === 1;
}
function fromProtocolCallHierarchyItem(item) {
    const useFileName = isSourceFileItem(item);
    const name = useFileName ? path.basename(item.file) : item.name;
    const detail = useFileName ? vscode.workspace.asRelativePath(path.dirname(item.file)) : '';
    return new vscode.CallHierarchyItem(typeConverters.SymbolKind.fromProtocolScriptElementKind(item.kind), name, detail, vscode.Uri.file(item.file), typeConverters.Range.fromTextSpan(item.span), typeConverters.Range.fromTextSpan(item.selectionSpan));
}
function fromProtocolCallHierchyIncomingCall(item) {
    return new vscode.CallHierarchyIncomingCall(fromProtocolCallHierarchyItem(item.from), item.fromSpans.map(typeConverters.Range.fromTextSpan));
}
function fromProtocolCallHierchyOutgoingCall(item) {
    return new vscode.CallHierarchyOutgoingCall(fromProtocolCallHierarchyItem(item.to), item.fromSpans.map(typeConverters.Range.fromTextSpan));
}
function register(selector, client) {
    return new dependentRegistration_1.VersionDependentRegistration(client, TypeScriptCallHierarchySupport.minVersion, () => vscode.languages.registerCallHierarchyProvider(selector, new TypeScriptCallHierarchySupport(client)));
}
exports.register = register;
//# sourceMappingURL=callHierarchy.js.map