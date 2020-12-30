"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const vscode = require("vscode");
const vscode_nls_1 = require("vscode-nls");
const localize = vscode_nls_1.loadMessageBundle();
class MergeConflictCodeLensProvider {
    constructor(trackerService) {
        this.tracker = trackerService.createTracker('codelens');
    }
    begin(config) {
        this.config = config;
        if (this.config.enableCodeLens) {
            this.registerCodeLensProvider();
        }
    }
    configurationUpdated(updatedConfig) {
        if (updatedConfig.enableCodeLens === false && this.codeLensRegistrationHandle) {
            this.codeLensRegistrationHandle.dispose();
            this.codeLensRegistrationHandle = null;
        }
        else if (updatedConfig.enableCodeLens === true && !this.codeLensRegistrationHandle) {
            this.registerCodeLensProvider();
        }
        this.config = updatedConfig;
    }
    dispose() {
        if (this.codeLensRegistrationHandle) {
            this.codeLensRegistrationHandle.dispose();
            this.codeLensRegistrationHandle = null;
        }
    }
    async provideCodeLenses(document, _token) {
        if (!this.config || !this.config.enableCodeLens) {
            return null;
        }
        let conflicts = await this.tracker.getConflicts(document);
        if (!conflicts || conflicts.length === 0) {
            return null;
        }
        let items = [];
        conflicts.forEach(conflict => {
            let acceptCurrentCommand = {
                command: 'merge-conflict.accept.current',
                title: localize('acceptCurrentChange', 'Accept Current Change'),
                arguments: ['known-conflict', conflict]
            };
            let acceptIncomingCommand = {
                command: 'merge-conflict.accept.incoming',
                title: localize('acceptIncomingChange', 'Accept Incoming Change'),
                arguments: ['known-conflict', conflict]
            };
            let acceptBothCommand = {
                command: 'merge-conflict.accept.both',
                title: localize('acceptBothChanges', 'Accept Both Changes'),
                arguments: ['known-conflict', conflict]
            };
            let diffCommand = {
                command: 'merge-conflict.compare',
                title: localize('compareChanges', 'Compare Changes'),
                arguments: [conflict]
            };
            items.push(new vscode.CodeLens(conflict.range, acceptCurrentCommand), new vscode.CodeLens(conflict.range.with(conflict.range.start.with({ character: conflict.range.start.character + 1 })), acceptIncomingCommand), new vscode.CodeLens(conflict.range.with(conflict.range.start.with({ character: conflict.range.start.character + 2 })), acceptBothCommand), new vscode.CodeLens(conflict.range.with(conflict.range.start.with({ character: conflict.range.start.character + 3 })), diffCommand));
        });
        return items;
    }
    registerCodeLensProvider() {
        this.codeLensRegistrationHandle = vscode.languages.registerCodeLensProvider([
            { scheme: 'file' },
            { scheme: 'untitled' },
            { scheme: 'vscode-userdata' },
        ], this);
    }
}
exports.default = MergeConflictCodeLensProvider;
//# sourceMappingURL=codelensProvider.js.map