"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.register = void 0;
const vscode = require("vscode");
const typescriptService_1 = require("../typescriptService");
const dependentRegistration_1 = require("../utils/dependentRegistration");
const Previewer = require("../utils/previewer");
const typeConverters = require("../utils/typeConverters");
class TypeScriptSignatureHelpProvider {
    constructor(client) {
        this.client = client;
    }
    async provideSignatureHelp(document, position, token, context) {
        const filepath = this.client.toOpenedFilePath(document);
        if (!filepath) {
            return undefined;
        }
        const args = {
            ...typeConverters.Position.toFileLocationRequestArgs(filepath, position),
            triggerReason: toTsTriggerReason(context)
        };
        const response = await this.client.interruptGetErr(() => this.client.execute('signatureHelp', args, token));
        if (response.type !== 'response' || !response.body) {
            return undefined;
        }
        const info = response.body;
        const result = new vscode.SignatureHelp();
        result.signatures = info.items.map(signature => this.convertSignature(signature));
        result.activeSignature = this.getActiveSignature(context, info, result.signatures);
        result.activeParameter = this.getActiveParameter(info);
        return result;
    }
    getActiveSignature(context, info, signatures) {
        var _a;
        // Try matching the previous active signature's label to keep it selected
        const previouslyActiveSignature = (_a = context.activeSignatureHelp) === null || _a === void 0 ? void 0 : _a.signatures[context.activeSignatureHelp.activeSignature];
        if (previouslyActiveSignature && context.isRetrigger) {
            const existingIndex = signatures.findIndex(other => other.label === (previouslyActiveSignature === null || previouslyActiveSignature === void 0 ? void 0 : previouslyActiveSignature.label));
            if (existingIndex >= 0) {
                return existingIndex;
            }
        }
        return info.selectedItemIndex;
    }
    getActiveParameter(info) {
        const activeSignature = info.items[info.selectedItemIndex];
        if (activeSignature && activeSignature.isVariadic) {
            return Math.min(info.argumentIndex, activeSignature.parameters.length - 1);
        }
        return info.argumentIndex;
    }
    convertSignature(item) {
        const signature = new vscode.SignatureInformation(Previewer.plain(item.prefixDisplayParts), Previewer.markdownDocumentation(item.documentation, item.tags.filter(x => x.name !== 'param')));
        let textIndex = signature.label.length;
        const separatorLabel = Previewer.plain(item.separatorDisplayParts);
        for (let i = 0; i < item.parameters.length; ++i) {
            const parameter = item.parameters[i];
            const label = Previewer.plain(parameter.displayParts);
            signature.parameters.push(new vscode.ParameterInformation([textIndex, textIndex + label.length], Previewer.markdownDocumentation(parameter.documentation, [])));
            textIndex += label.length;
            signature.label += label;
            if (i !== item.parameters.length - 1) {
                signature.label += separatorLabel;
                textIndex += separatorLabel.length;
            }
        }
        signature.label += Previewer.plain(item.suffixDisplayParts);
        return signature;
    }
}
TypeScriptSignatureHelpProvider.triggerCharacters = ['(', ',', '<'];
TypeScriptSignatureHelpProvider.retriggerCharacters = [')'];
function toTsTriggerReason(context) {
    switch (context.triggerKind) {
        case vscode.SignatureHelpTriggerKind.TriggerCharacter:
            if (context.triggerCharacter) {
                if (context.isRetrigger) {
                    return { kind: 'retrigger', triggerCharacter: context.triggerCharacter };
                }
                else {
                    return { kind: 'characterTyped', triggerCharacter: context.triggerCharacter };
                }
            }
            else {
                return { kind: 'invoked' };
            }
        case vscode.SignatureHelpTriggerKind.ContentChange:
            return context.isRetrigger ? { kind: 'retrigger' } : { kind: 'invoked' };
        case vscode.SignatureHelpTriggerKind.Invoke:
        default:
            return { kind: 'invoked' };
    }
}
function register(selector, client) {
    return dependentRegistration_1.conditionalRegistration([
        dependentRegistration_1.requireSomeCapability(client, typescriptService_1.ClientCapability.EnhancedSyntax, typescriptService_1.ClientCapability.Semantic),
    ], () => {
        return vscode.languages.registerSignatureHelpProvider(selector.syntax, new TypeScriptSignatureHelpProvider(client), {
            triggerCharacters: TypeScriptSignatureHelpProvider.triggerCharacters,
            retriggerCharacters: TypeScriptSignatureHelpProvider.retriggerCharacters
        });
    });
}
exports.register = register;
//# sourceMappingURL=signatureHelp.js.map