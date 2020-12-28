"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.register = void 0;
const vscode = require("vscode");
const fileSchemes = require("../utils/fileSchemes");
const languageDescription_1 = require("../utils/languageDescription");
const typeConverters = require("../utils/typeConverters");
const PConst = require("../protocol.const");
function getSymbolKind(item) {
    switch (item.kind) {
        case PConst.Kind.method: return vscode.SymbolKind.Method;
        case PConst.Kind.enum: return vscode.SymbolKind.Enum;
        case PConst.Kind.enumMember: return vscode.SymbolKind.EnumMember;
        case PConst.Kind.function: return vscode.SymbolKind.Function;
        case PConst.Kind.class: return vscode.SymbolKind.Class;
        case PConst.Kind.interface: return vscode.SymbolKind.Interface;
        case PConst.Kind.type: return vscode.SymbolKind.Class;
        case PConst.Kind.memberVariable: return vscode.SymbolKind.Field;
        case PConst.Kind.memberGetAccessor: return vscode.SymbolKind.Field;
        case PConst.Kind.memberSetAccessor: return vscode.SymbolKind.Field;
        case PConst.Kind.variable: return vscode.SymbolKind.Variable;
        default: return vscode.SymbolKind.Variable;
    }
}
class TypeScriptWorkspaceSymbolProvider {
    constructor(client, modeIds) {
        this.client = client;
        this.modeIds = modeIds;
    }
    async provideWorkspaceSymbols(search, token) {
        const document = this.getDocument();
        if (!document) {
            return [];
        }
        const filepath = await this.toOpenedFiledPath(document);
        if (!filepath) {
            return [];
        }
        const args = {
            file: filepath,
            searchValue: search,
            maxResultCount: 256,
        };
        const response = await this.client.execute('navto', args, token);
        if (response.type !== 'response' || !response.body) {
            return [];
        }
        return response.body
            .filter(item => item.containerName || item.kind !== 'alias')
            .map(item => this.toSymbolInformation(item));
    }
    async toOpenedFiledPath(document) {
        var _a;
        if (document.uri.scheme === fileSchemes.git) {
            try {
                const path = vscode.Uri.file((_a = JSON.parse(document.uri.query)) === null || _a === void 0 ? void 0 : _a.path);
                if (languageDescription_1.doesResourceLookLikeATypeScriptFile(path) || languageDescription_1.doesResourceLookLikeAJavaScriptFile(path)) {
                    const document = await vscode.workspace.openTextDocument(path);
                    return this.client.toOpenedFilePath(document);
                }
            }
            catch (_b) {
                // noop
            }
        }
        return this.client.toOpenedFilePath(document);
    }
    toSymbolInformation(item) {
        const label = TypeScriptWorkspaceSymbolProvider.getLabel(item);
        return new vscode.SymbolInformation(label, getSymbolKind(item), item.containerName || '', typeConverters.Location.fromTextSpan(this.client.toResource(item.file), item));
    }
    static getLabel(item) {
        const label = item.name;
        if (item.kind === 'method' || item.kind === 'function') {
            return label + '()';
        }
        return label;
    }
    getDocument() {
        // typescript wants to have a resource even when asking
        // general questions so we check the active editor. If this
        // doesn't match we take the first TS document.
        var _a;
        const activeDocument = (_a = vscode.window.activeTextEditor) === null || _a === void 0 ? void 0 : _a.document;
        if (activeDocument) {
            if (this.modeIds.includes(activeDocument.languageId)) {
                return activeDocument;
            }
        }
        const documents = vscode.workspace.textDocuments;
        for (const document of documents) {
            if (this.modeIds.includes(document.languageId)) {
                return document;
            }
        }
        return undefined;
    }
}
function register(client, modeIds) {
    return vscode.languages.registerWorkspaceSymbolProvider(new TypeScriptWorkspaceSymbolProvider(client, modeIds));
}
exports.register = register;
//# sourceMappingURL=workspaceSymbols.js.map