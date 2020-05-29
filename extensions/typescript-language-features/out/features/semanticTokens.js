"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.register = void 0;
const vscode = require("vscode");
const dependentRegistration_1 = require("../utils/dependentRegistration");
const api_1 = require("../utils/api");
const minTypeScriptVersion = api_1.default.fromVersionString(`${3 /* major */}.${7 /* minor */}`);
function register(selector, client) {
    return new dependentRegistration_1.VersionDependentRegistration(client, minTypeScriptVersion, () => {
        const provider = new DocumentSemanticTokensProvider(client);
        return vscode.Disposable.from(vscode.languages.registerDocumentSemanticTokensProvider(selector, provider, provider.getLegend()), vscode.languages.registerDocumentRangeSemanticTokensProvider(selector, provider, provider.getLegend()));
    });
}
exports.register = register;
// as we don't do deltas, for performance reasons, don't compute semantic tokens for documents above that limit
const CONTENT_LENGTH_LIMIT = 100000;
/**
 * Prototype of a DocumentSemanticTokensProvider, relying on the experimental `encodedSemanticClassifications-full` request from the TypeScript server.
 * As the results retured by the TypeScript server are limited, we also add a Typescript plugin (typescript-vscode-sh-plugin) to enrich the returned token.
 * See https://github.com/aeschli/typescript-vscode-sh-plugin.
 */
class DocumentSemanticTokensProvider {
    constructor(client) {
        this.client = client;
    }
    getLegend() {
        return new vscode.SemanticTokensLegend(tokenTypes, tokenModifiers);
    }
    async provideDocumentSemanticTokens(document, token) {
        const file = this.client.toOpenedFilePath(document);
        if (!file || document.getText().length > CONTENT_LENGTH_LIMIT) {
            return null;
        }
        return this._provideSemanticTokens(document, { file, start: 0, length: document.getText().length }, token);
    }
    async provideDocumentRangeSemanticTokens(document, range, token) {
        const file = this.client.toOpenedFilePath(document);
        if (!file || document.getText().length > CONTENT_LENGTH_LIMIT) {
            return null;
        }
        const start = document.offsetAt(range.start);
        const length = document.offsetAt(range.end) - start;
        return this._provideSemanticTokens(document, { file, start, length }, token);
    }
    async _provideSemanticTokens(document, requestArg, token) {
        const file = this.client.toOpenedFilePath(document);
        if (!file) {
            return null;
        }
        let versionBeforeRequest = document.version;
        const response = await this.client.execute('encodedSemanticClassifications-full', requestArg, token);
        if (response.type !== 'response' || !response.body) {
            return null;
        }
        const versionAfterRequest = document.version;
        if (versionBeforeRequest !== versionAfterRequest) {
            // cannot convert result's offsets to (line;col) values correctly
            // a new request will come in soon...
            //
            // here we cannot return null, because returning null would remove all semantic tokens.
            // we must throw to indicate that the semantic tokens should not be removed.
            // using the string busy here because it is not logged to error telemetry if the error text contains busy.
            // as the new request will come in right after our response, we first wait for the document activity to stop
            await waitForDocumentChangesToEnd(document);
            throw new Error('busy');
        }
        const tokenSpan = response.body.spans;
        const builder = new vscode.SemanticTokensBuilder();
        let i = 0;
        while (i < tokenSpan.length) {
            const offset = tokenSpan[i++];
            const length = tokenSpan[i++];
            const tsClassification = tokenSpan[i++];
            let tokenModifiers = 0;
            let tokenType = getTokenTypeFromClassification(tsClassification);
            if (tokenType !== undefined) {
                // it's a classification as returned by the typescript-vscode-sh-plugin
                tokenModifiers = getTokenModifierFromClassification(tsClassification);
            }
            else {
                // typescript-vscode-sh-plugin is not present
                tokenType = tokenTypeMap[tsClassification];
                if (tokenType === undefined) {
                    continue;
                }
            }
            // we can use the document's range conversion methods because the result is at the same version as the document
            const startPos = document.positionAt(offset);
            const endPos = document.positionAt(offset + length);
            for (let line = startPos.line; line <= endPos.line; line++) {
                const startCharacter = (line === startPos.line ? startPos.character : 0);
                const endCharacter = (line === endPos.line ? endPos.character : document.lineAt(line).text.length);
                builder.push(line, startCharacter, endCharacter - startCharacter, tokenType, tokenModifiers);
            }
        }
        return builder.build();
    }
}
function waitForDocumentChangesToEnd(document) {
    let version = document.version;
    return new Promise((s) => {
        let iv = setInterval(_ => {
            if (document.version === version) {
                clearInterval(iv);
                s();
            }
            version = document.version;
        }, 400);
    });
}
// typescript-vscode-sh-plugin encodes type and modifiers in the classification:
// TSClassification = (TokenType + 1) << 8 + TokenModifier
function getTokenTypeFromClassification(tsClassification) {
    if (tsClassification > 255 /* modifierMask */) {
        return (tsClassification >> 8 /* typeOffset */) - 1;
    }
    return undefined;
}
function getTokenModifierFromClassification(tsClassification) {
    return tsClassification & 255 /* modifierMask */;
}
const tokenTypes = [];
tokenTypes[0 /* class */] = 'class';
tokenTypes[1 /* enum */] = 'enum';
tokenTypes[2 /* interface */] = 'interface';
tokenTypes[3 /* namespace */] = 'namespace';
tokenTypes[4 /* typeParameter */] = 'typeParameter';
tokenTypes[5 /* type */] = 'type';
tokenTypes[6 /* parameter */] = 'parameter';
tokenTypes[7 /* variable */] = 'variable';
tokenTypes[8 /* enumMember */] = 'enumMember';
tokenTypes[9 /* property */] = 'property';
tokenTypes[10 /* function */] = 'function';
tokenTypes[11 /* member */] = 'member';
const tokenModifiers = [];
tokenModifiers[2 /* async */] = 'async';
tokenModifiers[0 /* declaration */] = 'declaration';
tokenModifiers[3 /* readonly */] = 'readonly';
tokenModifiers[1 /* static */] = 'static';
tokenModifiers[5 /* local */] = 'local';
tokenModifiers[4 /* defaultLibrary */] = 'defaultLibrary';
// make sure token types and modifiers are complete
if (tokenTypes.filter(t => !!t).length !== 12 /* _ */) {
    console.warn('typescript-vscode-sh-plugin has added new tokens types.');
}
if (tokenModifiers.filter(t => !!t).length !== 6 /* _ */) {
    console.warn('typescript-vscode-sh-plugin has added new tokens modifiers.');
}
// mapping for the original ExperimentalProtocol.ClassificationType from TypeScript (only used when plugin is not available)
const tokenTypeMap = [];
tokenTypeMap[11 /* className */] = 0 /* class */;
tokenTypeMap[12 /* enumName */] = 1 /* enum */;
tokenTypeMap[13 /* interfaceName */] = 2 /* interface */;
tokenTypeMap[14 /* moduleName */] = 3 /* namespace */;
tokenTypeMap[15 /* typeParameterName */] = 4 /* typeParameter */;
tokenTypeMap[16 /* typeAliasName */] = 5 /* type */;
tokenTypeMap[17 /* parameterName */] = 6 /* parameter */;
//# sourceMappingURL=semanticTokens.js.map