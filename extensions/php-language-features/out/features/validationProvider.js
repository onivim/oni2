"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.LineDecoder = void 0;
const cp = require("child_process");
const string_decoder_1 = require("string_decoder");
const vscode = require("vscode");
const async_1 = require("./utils/async");
const nls = require("vscode-nls");
let localize = nls.loadMessageBundle();
class LineDecoder {
    constructor(encoding = 'utf8') {
        this.stringDecoder = new string_decoder_1.StringDecoder(encoding);
        this.remaining = null;
    }
    write(buffer) {
        let result = [];
        let value = this.remaining
            ? this.remaining + this.stringDecoder.write(buffer)
            : this.stringDecoder.write(buffer);
        if (value.length < 1) {
            return result;
        }
        let start = 0;
        let ch;
        while (start < value.length && ((ch = value.charCodeAt(start)) === 13 || ch === 10)) {
            start++;
        }
        let idx = start;
        while (idx < value.length) {
            ch = value.charCodeAt(idx);
            if (ch === 13 || ch === 10) {
                result.push(value.substring(start, idx));
                idx++;
                while (idx < value.length && ((ch = value.charCodeAt(idx)) === 13 || ch === 10)) {
                    idx++;
                }
                start = idx;
            }
            else {
                idx++;
            }
        }
        this.remaining = start < value.length ? value.substr(start) : null;
        return result;
    }
    end() {
        return this.remaining;
    }
}
exports.LineDecoder = LineDecoder;
var RunTrigger;
(function (RunTrigger) {
    RunTrigger[RunTrigger["onSave"] = 0] = "onSave";
    RunTrigger[RunTrigger["onType"] = 1] = "onType";
})(RunTrigger || (RunTrigger = {}));
(function (RunTrigger) {
    RunTrigger.strings = {
        onSave: 'onSave',
        onType: 'onType'
    };
    RunTrigger.from = function (value) {
        if (value === 'onType') {
            return RunTrigger.onType;
        }
        else {
            return RunTrigger.onSave;
        }
    };
})(RunTrigger || (RunTrigger = {}));
class PHPValidationProvider {
    constructor(workspaceStore) {
        this.workspaceStore = workspaceStore;
        this.documentListener = null;
        this.executable = undefined;
        this.validationEnabled = true;
        this.trigger = RunTrigger.onSave;
        this.pauseValidation = false;
    }
    activate(subscriptions) {
        this.diagnosticCollection = vscode.languages.createDiagnosticCollection();
        subscriptions.push(this);
        vscode.workspace.onDidChangeConfiguration(this.loadConfiguration, this, subscriptions);
        this.loadConfiguration();
        vscode.workspace.onDidOpenTextDocument(this.triggerValidate, this, subscriptions);
        vscode.workspace.onDidCloseTextDocument((textDocument) => {
            this.diagnosticCollection.delete(textDocument.uri);
            delete this.delayers[textDocument.uri.toString()];
        }, null, subscriptions);
        subscriptions.push(vscode.commands.registerCommand('php.untrustValidationExecutable', this.untrustValidationExecutable, this));
    }
    dispose() {
        if (this.diagnosticCollection) {
            this.diagnosticCollection.clear();
            this.diagnosticCollection.dispose();
        }
        if (this.documentListener) {
            this.documentListener.dispose();
            this.documentListener = null;
        }
    }
    loadConfiguration() {
        let section = vscode.workspace.getConfiguration();
        let oldExecutable = this.executable;
        if (section) {
            this.validationEnabled = section.get("php.validate.enable" /* Enable */, true);
            let inspect = section.inspect("php.validate.executablePath" /* ExecutablePath */);
            if (inspect && inspect.workspaceValue) {
                this.executable = inspect.workspaceValue;
                this.executableIsUserDefined = false;
            }
            else if (inspect && inspect.globalValue) {
                this.executable = inspect.globalValue;
                this.executableIsUserDefined = true;
            }
            else {
                this.executable = undefined;
                this.executableIsUserDefined = undefined;
            }
            this.trigger = RunTrigger.from(section.get("php.validate.run" /* Run */, RunTrigger.strings.onSave));
        }
        if (this.executableIsUserDefined !== true && this.workspaceStore.get("php.validate.checkedExecutablePath" /* CheckedExecutablePath */, undefined) !== undefined) {
            vscode.commands.executeCommand('setContext', 'php.untrustValidationExecutableContext', true);
        }
        this.delayers = Object.create(null);
        if (this.pauseValidation) {
            this.pauseValidation = oldExecutable === this.executable;
        }
        if (this.documentListener) {
            this.documentListener.dispose();
            this.documentListener = null;
        }
        this.diagnosticCollection.clear();
        if (this.validationEnabled) {
            if (this.trigger === RunTrigger.onType) {
                this.documentListener = vscode.workspace.onDidChangeTextDocument((e) => {
                    this.triggerValidate(e.document);
                });
            }
            else {
                this.documentListener = vscode.workspace.onDidSaveTextDocument(this.triggerValidate, this);
            }
            // Configuration has changed. Reevaluate all documents.
            vscode.workspace.textDocuments.forEach(this.triggerValidate, this);
        }
    }
    untrustValidationExecutable() {
        this.workspaceStore.update("php.validate.checkedExecutablePath" /* CheckedExecutablePath */, undefined);
        vscode.commands.executeCommand('setContext', 'php.untrustValidationExecutableContext', false);
    }
    triggerValidate(textDocument) {
        if (textDocument.languageId !== 'php' || this.pauseValidation || !this.validationEnabled) {
            return;
        }
        let trigger = () => {
            let key = textDocument.uri.toString();
            let delayer = this.delayers[key];
            if (!delayer) {
                delayer = new async_1.ThrottledDelayer(this.trigger === RunTrigger.onType ? 250 : 0);
                this.delayers[key] = delayer;
            }
            delayer.trigger(() => this.doValidate(textDocument));
        };
        if (this.executableIsUserDefined !== undefined && !this.executableIsUserDefined) {
            let checkedExecutablePath = this.workspaceStore.get("php.validate.checkedExecutablePath" /* CheckedExecutablePath */, undefined);
            if (!checkedExecutablePath || checkedExecutablePath !== this.executable) {
                vscode.window.showInformationMessage(localize('php.useExecutablePath', 'Do you allow {0} (defined as a workspace setting) to be executed to lint PHP files?', this.executable), {
                    title: localize('php.yes', 'Allow'),
                    id: 'yes'
                }, {
                    title: localize('php.no', 'Disallow'),
                    isCloseAffordance: true,
                    id: 'no'
                }).then(selected => {
                    if (!selected || selected.id === 'no') {
                        this.pauseValidation = true;
                    }
                    else if (selected.id === 'yes') {
                        this.workspaceStore.update("php.validate.checkedExecutablePath" /* CheckedExecutablePath */, this.executable);
                        vscode.commands.executeCommand('setContext', 'php.untrustValidationExecutableContext', true);
                        trigger();
                    }
                });
                return;
            }
        }
        trigger();
    }
    doValidate(textDocument) {
        return new Promise((resolve) => {
            let executable = this.executable || 'php';
            let decoder = new LineDecoder();
            let diagnostics = [];
            let processLine = (line) => {
                let matches = line.match(PHPValidationProvider.MatchExpression);
                if (matches) {
                    let message = matches[1];
                    let line = parseInt(matches[3]) - 1;
                    let diagnostic = new vscode.Diagnostic(new vscode.Range(line, 0, line, Number.MAX_VALUE), message);
                    diagnostics.push(diagnostic);
                }
            };
            let options = (vscode.workspace.workspaceFolders && vscode.workspace.workspaceFolders[0]) ? { cwd: vscode.workspace.workspaceFolders[0].uri.fsPath } : undefined;
            let args;
            if (this.trigger === RunTrigger.onSave) {
                args = PHPValidationProvider.FileArgs.slice(0);
                args.push(textDocument.fileName);
            }
            else {
                args = PHPValidationProvider.BufferArgs;
            }
            try {
                let childProcess = cp.spawn(executable, args, options);
                childProcess.on('error', (error) => {
                    if (this.pauseValidation) {
                        resolve();
                        return;
                    }
                    this.showError(error, executable);
                    this.pauseValidation = true;
                    resolve();
                });
                if (childProcess.pid) {
                    if (this.trigger === RunTrigger.onType) {
                        childProcess.stdin.write(textDocument.getText());
                        childProcess.stdin.end();
                    }
                    childProcess.stdout.on('data', (data) => {
                        decoder.write(data).forEach(processLine);
                    });
                    childProcess.stdout.on('end', () => {
                        let line = decoder.end();
                        if (line) {
                            processLine(line);
                        }
                        this.diagnosticCollection.set(textDocument.uri, diagnostics);
                        resolve();
                    });
                }
                else {
                    resolve();
                }
            }
            catch (error) {
                this.showError(error, executable);
            }
        });
    }
    async showError(error, executable) {
        let message = null;
        if (error.code === 'ENOENT') {
            if (this.executable) {
                message = localize('wrongExecutable', 'Cannot validate since {0} is not a valid php executable. Use the setting \'php.validate.executablePath\' to configure the PHP executable.', executable);
            }
            else {
                message = localize('noExecutable', 'Cannot validate since no PHP executable is set. Use the setting \'php.validate.executablePath\' to configure the PHP executable.');
            }
        }
        else {
            message = error.message ? error.message : localize('unknownReason', 'Failed to run php using path: {0}. Reason is unknown.', executable);
        }
        if (!message) {
            return;
        }
        const openSettings = localize('goToSetting', 'Open Settings');
        if (await vscode.window.showInformationMessage(message, openSettings) === openSettings) {
            vscode.commands.executeCommand('workbench.action.openSettings', "php.validate.executablePath" /* ExecutablePath */);
        }
    }
}
exports.default = PHPValidationProvider;
PHPValidationProvider.MatchExpression = /(?:(?:Parse|Fatal) error): (.*)(?: in )(.*?)(?: on line )(\d+)/;
PHPValidationProvider.BufferArgs = ['-l', '-n', '-d', 'display_errors=On', '-d', 'log_errors=Off'];
PHPValidationProvider.FileArgs = ['-l', '-n', '-d', 'display_errors=On', '-d', 'log_errors=Off', '-f'];
//# sourceMappingURL=validationProvider.js.map