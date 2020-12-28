"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.NpmScriptsTreeDataProvider = void 0;
const jsonc_parser_1 = require("jsonc-parser");
const path = require("path");
const vscode_1 = require("vscode");
const nls = require("vscode-nls");
const tasks_1 = require("./tasks");
const localize = nls.loadMessageBundle();
class Folder extends vscode_1.TreeItem {
    constructor(folder) {
        super(folder.name, vscode_1.TreeItemCollapsibleState.Expanded);
        this.packages = [];
        this.contextValue = 'folder';
        this.resourceUri = folder.uri;
        this.workspaceFolder = folder;
        this.iconPath = vscode_1.ThemeIcon.Folder;
    }
    addPackage(packageJson) {
        this.packages.push(packageJson);
    }
}
const packageName = 'package.json';
class PackageJSON extends vscode_1.TreeItem {
    constructor(folder, relativePath) {
        super(PackageJSON.getLabel(relativePath), vscode_1.TreeItemCollapsibleState.Expanded);
        this.scripts = [];
        this.folder = folder;
        this.path = relativePath;
        this.contextValue = 'packageJSON';
        if (relativePath) {
            this.resourceUri = vscode_1.Uri.file(path.join(folder.resourceUri.fsPath, relativePath, packageName));
        }
        else {
            this.resourceUri = vscode_1.Uri.file(path.join(folder.resourceUri.fsPath, packageName));
        }
        this.iconPath = vscode_1.ThemeIcon.File;
    }
    static getLabel(relativePath) {
        if (relativePath.length > 0) {
            return path.join(relativePath, packageName);
        }
        return packageName;
    }
    addScript(script) {
        this.scripts.push(script);
    }
}
class NpmScript extends vscode_1.TreeItem {
    constructor(_context, packageJson, task, taskLocation) {
        super(task.name, vscode_1.TreeItemCollapsibleState.None);
        this.taskLocation = taskLocation;
        const command = vscode_1.workspace.getConfiguration('npm').get('scriptExplorerAction') || 'open';
        const commandList = {
            'open': {
                title: 'Edit Script',
                command: 'vscode.open',
                arguments: [
                    taskLocation === null || taskLocation === void 0 ? void 0 : taskLocation.document,
                    taskLocation ? {
                        selection: new vscode_1.Range(taskLocation.line, taskLocation.line)
                    } : undefined
                ]
            },
            'run': {
                title: 'Run Script',
                command: 'npm.runScript',
                arguments: [this]
            }
        };
        this.contextValue = 'script';
        this.package = packageJson;
        this.task = task;
        this.command = commandList[command];
        if (task.group && task.group === vscode_1.TaskGroup.Clean) {
            this.iconPath = new vscode_1.ThemeIcon('wrench-subaction');
        }
        else {
            this.iconPath = new vscode_1.ThemeIcon('wrench');
        }
        if (task.detail) {
            this.tooltip = task.detail;
        }
    }
    getFolder() {
        return this.package.folder.workspaceFolder;
    }
}
class NoScripts extends vscode_1.TreeItem {
    constructor(message) {
        super(message, vscode_1.TreeItemCollapsibleState.None);
        this.contextValue = 'noscripts';
    }
}
class NpmScriptsTreeDataProvider {
    constructor(context, taskProvider) {
        this.taskProvider = taskProvider;
        this.taskTree = null;
        this._onDidChangeTreeData = new vscode_1.EventEmitter();
        this.onDidChangeTreeData = this._onDidChangeTreeData.event;
        const subscriptions = context.subscriptions;
        this.extensionContext = context;
        subscriptions.push(vscode_1.commands.registerCommand('npm.runScript', this.runScript, this));
        subscriptions.push(vscode_1.commands.registerCommand('npm.debugScript', this.debugScript, this));
        subscriptions.push(vscode_1.commands.registerCommand('npm.openScript', this.openScript, this));
        subscriptions.push(vscode_1.commands.registerCommand('npm.runInstall', this.runInstall, this));
    }
    async runScript(script) {
        vscode_1.tasks.executeTask(script.task);
    }
    async debugScript(script) {
        tasks_1.startDebugging(this.extensionContext, script.task.definition.script, path.dirname(script.package.resourceUri.fsPath), script.getFolder());
    }
    findScript(document, script) {
        let scriptOffset = 0;
        let inScripts = false;
        let visitor = {
            onError() {
                return scriptOffset;
            },
            onObjectEnd() {
                if (inScripts) {
                    inScripts = false;
                }
            },
            onObjectProperty(property, offset, _length) {
                if (property === 'scripts') {
                    inScripts = true;
                    if (!script) { // select the script section
                        scriptOffset = offset;
                    }
                }
                else if (inScripts && script) {
                    let label = tasks_1.getTaskName(property, script.task.definition.path);
                    if (script.task.name === label) {
                        scriptOffset = offset;
                    }
                }
            }
        };
        jsonc_parser_1.visit(document.getText(), visitor);
        return scriptOffset;
    }
    async runInstall(selection) {
        let uri = undefined;
        if (selection instanceof PackageJSON) {
            uri = selection.resourceUri;
        }
        if (!uri) {
            return;
        }
        let task = await tasks_1.createTask(this.extensionContext, 'install', 'install', selection.folder.workspaceFolder, uri, undefined, []);
        vscode_1.tasks.executeTask(task);
    }
    async openScript(selection) {
        let uri = undefined;
        if (selection instanceof PackageJSON) {
            uri = selection.resourceUri;
        }
        else if (selection instanceof NpmScript) {
            uri = selection.package.resourceUri;
        }
        if (!uri) {
            return;
        }
        let document = await vscode_1.workspace.openTextDocument(uri);
        let offset = this.findScript(document, selection instanceof NpmScript ? selection : undefined);
        let position = document.positionAt(offset);
        await vscode_1.window.showTextDocument(document, { preserveFocus: true, selection: new vscode_1.Selection(position, position) });
    }
    refresh() {
        this.taskTree = null;
        this._onDidChangeTreeData.fire(null);
    }
    getTreeItem(element) {
        return element;
    }
    getParent(element) {
        if (element instanceof Folder) {
            return null;
        }
        if (element instanceof PackageJSON) {
            return element.folder;
        }
        if (element instanceof NpmScript) {
            return element.package;
        }
        if (element instanceof NoScripts) {
            return null;
        }
        return null;
    }
    async getChildren(element) {
        if (!this.taskTree) {
            const taskItems = await this.taskProvider.tasksWithLocation;
            if (taskItems) {
                this.taskTree = this.buildTaskTree(taskItems);
                if (this.taskTree.length === 0) {
                    let message = localize('noScripts', 'No scripts found.');
                    if (!tasks_1.isAutoDetectionEnabled()) {
                        message = localize('autoDetectIsOff', 'The setting "npm.autoDetect" is "off".');
                    }
                    this.taskTree = [new NoScripts(message)];
                }
            }
        }
        if (element instanceof Folder) {
            return element.packages;
        }
        if (element instanceof PackageJSON) {
            return element.scripts;
        }
        if (element instanceof NpmScript) {
            return [];
        }
        if (element instanceof NoScripts) {
            return [];
        }
        if (!element) {
            if (this.taskTree) {
                return this.taskTree;
            }
        }
        return [];
    }
    isInstallTask(task) {
        let fullName = tasks_1.getTaskName('install', task.definition.path);
        return fullName === task.name;
    }
    buildTaskTree(tasks) {
        let folders = new Map();
        let packages = new Map();
        let folder = null;
        let packageJson = null;
        tasks.forEach(each => {
            if (tasks_1.isWorkspaceFolder(each.task.scope) && !this.isInstallTask(each.task)) {
                folder = folders.get(each.task.scope.name);
                if (!folder) {
                    folder = new Folder(each.task.scope);
                    folders.set(each.task.scope.name, folder);
                }
                let definition = each.task.definition;
                let relativePath = definition.path ? definition.path : '';
                let fullPath = path.join(each.task.scope.name, relativePath);
                packageJson = packages.get(fullPath);
                if (!packageJson) {
                    packageJson = new PackageJSON(folder, relativePath);
                    folder.addPackage(packageJson);
                    packages.set(fullPath, packageJson);
                }
                let script = new NpmScript(this.extensionContext, packageJson, each.task, each.location);
                packageJson.addScript(script);
            }
        });
        if (folders.size === 1) {
            return [...packages.values()];
        }
        return [...folders.values()];
    }
}
exports.NpmScriptsTreeDataProvider = NpmScriptsTreeDataProvider;
//# sourceMappingURL=npmView.js.map