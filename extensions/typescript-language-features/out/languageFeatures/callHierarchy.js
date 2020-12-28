"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.register = void 0;
const path = require("path");
const vscode = require("vscode");
const PConst = require("../protocol.const");
const typescriptService_1 = require("../typescriptService");
const api_1 = require("../utils/api");
const dependentRegistration_1 = require("../utils/dependentRegistration");
const modifiers_1 = require("../utils/modifiers");
const typeConverters = require("../utils/typeConverters");
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
function isSourceFileItem(item) {
    return item.kind === PConst.Kind.script || item.kind === PConst.Kind.module && item.selectionSpan.start.line === 1 && item.selectionSpan.start.offset === 1;
}
function fromProtocolCallHierarchyItem(item) {
    var _a;
    const useFileName = isSourceFileItem(item);
    const name = useFileName ? path.basename(item.file) : item.name;
    const detail = useFileName ? vscode.workspace.asRelativePath(path.dirname(item.file)) : (_a = item.containerName) !== null && _a !== void 0 ? _a : '';
    const result = new vscode.CallHierarchyItem(typeConverters.SymbolKind.fromProtocolScriptElementKind(item.kind), name, detail, vscode.Uri.file(item.file), typeConverters.Range.fromTextSpan(item.span), typeConverters.Range.fromTextSpan(item.selectionSpan));
    const kindModifiers = item.kindModifiers ? modifiers_1.parseKindModifier(item.kindModifiers) : undefined;
    if (kindModifiers === null || kindModifiers === void 0 ? void 0 : kindModifiers.has(PConst.KindModifiers.depreacted)) {
        result.tags = [vscode.SymbolTag.Deprecated];
    }
    return result;
}
function fromProtocolCallHierchyIncomingCall(item) {
    return new vscode.CallHierarchyIncomingCall(fromProtocolCallHierarchyItem(item.from), item.fromSpans.map(typeConverters.Range.fromTextSpan));
}
function fromProtocolCallHierchyOutgoingCall(item) {
    return new vscode.CallHierarchyOutgoingCall(fromProtocolCallHierarchyItem(item.to), item.fromSpans.map(typeConverters.Range.fromTextSpan));
}
function register(selector, client) {
    return dependentRegistration_1.conditionalRegistration([
        dependentRegistration_1.requireMinVersion(client, TypeScriptCallHierarchySupport.minVersion),
        dependentRegistration_1.requireSomeCapability(client, typescriptService_1.ClientCapability.Semantic),
    ], () => {
        return vscode.languages.registerCallHierarchyProvider(selector.semantic, new TypeScriptCallHierarchySupport(client));
    });
}
exports.register = register;
//# sourceMappingURL=callHierarchy.js.map