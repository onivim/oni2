"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.getSemanticTokens = exports.getSemanticTokenLegend = void 0;
const ts = require("typescript");
function getSemanticTokenLegend() {
    if (tokenTypes.length !== 11 /* _ */) {
        console.warn('TokenType has added new entries.');
    }
    if (tokenModifiers.length !== 4 /* _ */) {
        console.warn('TokenModifier has added new entries.');
    }
    return { types: tokenTypes, modifiers: tokenModifiers };
}
exports.getSemanticTokenLegend = getSemanticTokenLegend;
function getSemanticTokens(jsLanguageService, currentTextDocument, fileName) {
    //https://ts-ast-viewer.com/#code/AQ0g2CmAuwGbALzAJwG4BQZQGNwEMBnQ4AQQEYBmYAb2C22zgEtJwATJVTRxgcwD27AQAp8AGmAAjAJS0A9POB8+7NQ168oscAJz5wANXwAnLug2bsJmAFcTAO2XAA1MHyvgu-UdOeWbOw8ViAAvpagocBAA
    let resultTokens = [];
    const collector = (node, typeIdx, modifierSet) => {
        resultTokens.push({ start: currentTextDocument.positionAt(node.getStart()), length: node.getWidth(), typeIdx, modifierSet });
    };
    collectTokens(jsLanguageService, fileName, { start: 0, length: currentTextDocument.getText().length }, collector);
    return resultTokens;
}
exports.getSemanticTokens = getSemanticTokens;
function collectTokens(jsLanguageService, fileName, span, collector) {
    const program = jsLanguageService.getProgram();
    if (program) {
        const typeChecker = program.getTypeChecker();
        function visit(node) {
            if (!node || !ts.textSpanIntersectsWith(span, node.pos, node.getFullWidth())) {
                return;
            }
            if (ts.isIdentifier(node)) {
                let symbol = typeChecker.getSymbolAtLocation(node);
                if (symbol) {
                    if (symbol.flags & ts.SymbolFlags.Alias) {
                        symbol = typeChecker.getAliasedSymbol(symbol);
                    }
                    let typeIdx = classifySymbol(symbol);
                    if (typeIdx !== undefined) {
                        let modifierSet = 0;
                        if (node.parent) {
                            const parentTypeIdx = tokenFromDeclarationMapping[node.parent.kind];
                            if (parentTypeIdx === typeIdx && node.parent.name === node) {
                                modifierSet = 1 << 0 /* declaration */;
                            }
                        }
                        const decl = symbol.valueDeclaration;
                        const modifiers = decl ? ts.getCombinedModifierFlags(decl) : 0;
                        const nodeFlags = decl ? ts.getCombinedNodeFlags(decl) : 0;
                        if (modifiers & ts.ModifierFlags.Static) {
                            modifierSet |= 1 << 1 /* static */;
                        }
                        if (modifiers & ts.ModifierFlags.Async) {
                            modifierSet |= 1 << 2 /* async */;
                        }
                        if ((modifiers & ts.ModifierFlags.Readonly) || (nodeFlags & ts.NodeFlags.Const) || (symbol.getFlags() & ts.SymbolFlags.EnumMember)) {
                            modifierSet |= 1 << 3 /* readonly */;
                        }
                        collector(node, typeIdx, modifierSet);
                    }
                }
            }
            ts.forEachChild(node, visit);
        }
        const sourceFile = program.getSourceFile(fileName);
        if (sourceFile) {
            visit(sourceFile);
        }
    }
}
function classifySymbol(symbol) {
    const flags = symbol.getFlags();
    if (flags & ts.SymbolFlags.Class) {
        return 0 /* class */;
    }
    else if (flags & ts.SymbolFlags.Enum) {
        return 1 /* enum */;
    }
    else if (flags & ts.SymbolFlags.TypeAlias) {
        return 5 /* type */;
    }
    else if (flags & ts.SymbolFlags.Type) {
        if (flags & ts.SymbolFlags.Interface) {
            return 2 /* interface */;
        }
        if (flags & ts.SymbolFlags.TypeParameter) {
            return 4 /* typeParameter */;
        }
    }
    const decl = symbol.valueDeclaration || symbol.declarations && symbol.declarations[0];
    return decl && tokenFromDeclarationMapping[decl.kind];
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
tokenTypes[8 /* property */] = 'property';
tokenTypes[9 /* function */] = 'function';
tokenTypes[10 /* method */] = 'method';
const tokenModifiers = [];
tokenModifiers[2 /* async */] = 'async';
tokenModifiers[0 /* declaration */] = 'declaration';
tokenModifiers[3 /* readonly */] = 'readonly';
tokenModifiers[1 /* static */] = 'static';
const tokenFromDeclarationMapping = {
    [ts.SyntaxKind.VariableDeclaration]: 7 /* variable */,
    [ts.SyntaxKind.Parameter]: 6 /* parameter */,
    [ts.SyntaxKind.PropertyDeclaration]: 8 /* property */,
    [ts.SyntaxKind.ModuleDeclaration]: 3 /* namespace */,
    [ts.SyntaxKind.EnumDeclaration]: 1 /* enum */,
    [ts.SyntaxKind.EnumMember]: 8 /* property */,
    [ts.SyntaxKind.ClassDeclaration]: 0 /* class */,
    [ts.SyntaxKind.MethodDeclaration]: 10 /* method */,
    [ts.SyntaxKind.FunctionDeclaration]: 9 /* function */,
    [ts.SyntaxKind.MethodSignature]: 10 /* method */,
    [ts.SyntaxKind.GetAccessor]: 8 /* property */,
    [ts.SyntaxKind.PropertySignature]: 8 /* property */,
    [ts.SyntaxKind.InterfaceDeclaration]: 2 /* interface */,
    [ts.SyntaxKind.TypeAliasDeclaration]: 5 /* type */,
    [ts.SyntaxKind.TypeParameter]: 4 /* typeParameter */
};
//# sourceMappingURL=javascriptSemanticTokens.js.map