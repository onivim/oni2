"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.LearnMoreAboutRefactoringsCommand = void 0;
const vscode = require("vscode");
const languageModeIds_1 = require("../utils/languageModeIds");
let LearnMoreAboutRefactoringsCommand = /** @class */ (() => {
    class LearnMoreAboutRefactoringsCommand {
        constructor() {
            this.id = LearnMoreAboutRefactoringsCommand.id;
        }
        execute() {
            const docUrl = vscode.window.activeTextEditor && languageModeIds_1.isTypeScriptDocument(vscode.window.activeTextEditor.document)
                ? 'https://go.microsoft.com/fwlink/?linkid=2114477'
                : 'https://go.microsoft.com/fwlink/?linkid=2116761';
            vscode.env.openExternal(vscode.Uri.parse(docUrl));
        }
    }
    LearnMoreAboutRefactoringsCommand.id = '_typescript.learnMoreAboutRefactorings';
    return LearnMoreAboutRefactoringsCommand;
})();
exports.LearnMoreAboutRefactoringsCommand = LearnMoreAboutRefactoringsCommand;
//# sourceMappingURL=learnMoreAboutRefactorings.js.map