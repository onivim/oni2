"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
const languageModeIds = require("./languageModeIds");
exports.allDiagnosticLangauges = [0 /* JavaScript */, 1 /* TypeScript */];
exports.standardLanguageDescriptions = [
    {
        id: 'typescript',
        diagnosticOwner: 'typescript',
        diagnosticSource: 'ts',
        diagnosticLanguage: 1 /* TypeScript */,
        modeIds: [languageModeIds.typescript, languageModeIds.typescriptreact],
        configFilePattern: /^tsconfig(\..*)?\.json$/gi
    }, {
        id: 'javascript',
        diagnosticOwner: 'typescript',
        diagnosticSource: 'ts',
        diagnosticLanguage: 0 /* JavaScript */,
        modeIds: [languageModeIds.javascript, languageModeIds.javascriptreact],
        configFilePattern: /^jsconfig(\..*)?\.json$/gi
    }
];
//# sourceMappingURL=languageDescription.js.map