"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
const vscode = require("vscode");
const documentTracker_1 = require("./documentTracker");
const codelensProvider_1 = require("./codelensProvider");
const commandHandler_1 = require("./commandHandler");
const contentProvider_1 = require("./contentProvider");
const mergeDecorator_1 = require("./mergeDecorator");
const ConfigurationSectionName = 'merge-conflict';
class ServiceWrapper {
    constructor(context) {
        this.context = context;
        this.services = [];
    }
    begin() {
        let configuration = this.createExtensionConfiguration();
        const documentTracker = new documentTracker_1.default();
        this.services.push(documentTracker, new commandHandler_1.default(documentTracker), new codelensProvider_1.default(documentTracker), new contentProvider_1.default(this.context), new mergeDecorator_1.default(this.context, documentTracker));
        this.services.forEach((service) => {
            if (service.begin && service.begin instanceof Function) {
                service.begin(configuration);
            }
        });
        vscode.workspace.onDidChangeConfiguration(() => {
            this.services.forEach((service) => {
                if (service.configurationUpdated && service.configurationUpdated instanceof Function) {
                    service.configurationUpdated(this.createExtensionConfiguration());
                }
            });
        });
    }
    createExtensionConfiguration() {
        const workspaceConfiguration = vscode.workspace.getConfiguration(ConfigurationSectionName);
        const codeLensEnabled = workspaceConfiguration.get('codeLens.enabled', true);
        const decoratorsEnabled = workspaceConfiguration.get('decorators.enabled', true);
        return {
            enableCodeLens: codeLensEnabled,
            enableDecorations: decoratorsEnabled,
            enableEditorOverview: decoratorsEnabled
        };
    }
    dispose() {
        this.services.forEach(disposable => disposable.dispose());
        this.services = [];
    }
}
exports.default = ServiceWrapper;
//# sourceMappingURL=services.js.map