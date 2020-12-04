"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.TerminalEnvironmentManager = void 0;
const vscode_1 = require("vscode");
const util_1 = require("./util");
class TerminalEnvironmentManager {
    constructor(context, env) {
        this.context = context;
        this.env = env;
        this._enabled = false;
        this.disposable = util_1.filterEvent(vscode_1.workspace.onDidChangeConfiguration, e => e.affectsConfiguration('git'))(this.refresh, this);
        this.refresh();
    }
    set enabled(enabled) {
        if (this._enabled === enabled) {
            return;
        }
        this._enabled = enabled;
        this.context.environmentVariableCollection.clear();
        if (enabled) {
            for (const name of Object.keys(this.env)) {
                this.context.environmentVariableCollection.replace(name, this.env[name]);
            }
        }
    }
    refresh() {
        const config = vscode_1.workspace.getConfiguration('git', null);
        this.enabled = config.get('enabled', true) && config.get('terminalAuthentication', true);
    }
    dispose() {
        this.disposable.dispose();
    }
}
exports.TerminalEnvironmentManager = TerminalEnvironmentManager;
//# sourceMappingURL=terminal.js.map