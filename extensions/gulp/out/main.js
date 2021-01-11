"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.deactivate = exports.activate = void 0;
const path = require("path");
const fs = require("fs");
const cp = require("child_process");
const vscode = require("vscode");
const nls = require("vscode-nls");
const localize = nls.loadMessageBundle();
function exists(file) {
    return new Promise((resolve, _reject) => {
        fs.exists(file, (value) => {
            resolve(value);
        });
    });
}
function exec(command, options) {
    return new Promise((resolve, reject) => {
        cp.exec(command, options, (error, stdout, stderr) => {
            if (error) {
                reject({ error, stdout, stderr });
            }
            resolve({ stdout, stderr });
        });
    });
}
const buildNames = ['build', 'compile', 'watch'];
function isBuildTask(name) {
    for (let buildName of buildNames) {
        if (name.indexOf(buildName) !== -1) {
            return true;
        }
    }
    return false;
}
const testNames = ['test'];
function isTestTask(name) {
    for (let testName of testNames) {
        if (name.indexOf(testName) !== -1) {
            return true;
        }
    }
    return false;
}
let _channel;
function getOutputChannel() {
    if (!_channel) {
        _channel = vscode.window.createOutputChannel('Gulp Auto Detection');
    }
    return _channel;
}
function showError() {
    vscode.window.showWarningMessage(localize('gulpTaskDetectError', 'Problem finding gulp tasks. See the output for more information.'), localize('gulpShowOutput', 'Go to output')).then((choice) => {
        if (choice !== undefined) {
            _channel.show(true);
        }
    });
}
async function findGulpCommand(rootPath) {
    let gulpCommand;
    let platform = process.platform;
    if (platform === 'win32' && await exists(path.join(rootPath, 'node_modules', '.bin', 'gulp.cmd'))) {
        const globalGulp = path.join(process.env.APPDATA ? process.env.APPDATA : '', 'npm', 'gulp.cmd');
        if (await exists(globalGulp)) {
            gulpCommand = '"' + globalGulp + '"';
        }
        else {
            gulpCommand = path.join('.', 'node_modules', '.bin', 'gulp.cmd');
        }
    }
    else if ((platform === 'linux' || platform === 'darwin') && await exists(path.join(rootPath, 'node_modules', '.bin', 'gulp'))) {
        gulpCommand = path.join('.', 'node_modules', '.bin', 'gulp');
    }
    else {
        gulpCommand = 'gulp';
    }
    return gulpCommand;
}
class FolderDetector {
    constructor(_workspaceFolder, _gulpCommand) {
        this._workspaceFolder = _workspaceFolder;
        this._gulpCommand = _gulpCommand;
    }
    get workspaceFolder() {
        return this._workspaceFolder;
    }
    isEnabled() {
        return vscode.workspace.getConfiguration('gulp', this._workspaceFolder.uri).get('autoDetect') === 'on';
    }
    start() {
        let pattern = path.join(this._workspaceFolder.uri.fsPath, '{node_modules,gulpfile{.babel.js,.js,.ts}}');
        this.fileWatcher = vscode.workspace.createFileSystemWatcher(pattern);
        this.fileWatcher.onDidChange(() => this.promise = undefined);
        this.fileWatcher.onDidCreate(() => this.promise = undefined);
        this.fileWatcher.onDidDelete(() => this.promise = undefined);
    }
    async getTasks() {
        if (this.isEnabled()) {
            if (!this.promise) {
                this.promise = this.computeTasks();
            }
            return this.promise;
        }
        else {
            return [];
        }
    }
    async getTask(_task) {
        const gulpTask = _task.definition.task;
        if (gulpTask) {
            let kind = _task.definition;
            let options = { cwd: this.workspaceFolder.uri.fsPath };
            let task = new vscode.Task(kind, this.workspaceFolder, gulpTask, 'gulp', new vscode.ShellExecution(await this._gulpCommand, [gulpTask], options));
            return task;
        }
        return undefined;
    }
    async computeTasks() {
        let rootPath = this._workspaceFolder.uri.scheme === 'file' ? this._workspaceFolder.uri.fsPath : undefined;
        let emptyTasks = [];
        if (!rootPath) {
            return emptyTasks;
        }
        let gulpfile = path.join(rootPath, 'gulpfile.js');
        if (!await exists(gulpfile)) {
            gulpfile = path.join(rootPath, 'Gulpfile.js');
            if (!await exists(gulpfile)) {
                gulpfile = path.join(rootPath, 'gulpfile.babel.js');
                if (!await exists(gulpfile)) {
                    return emptyTasks;
                }
            }
        }
        let commandLine = `${await this._gulpCommand} --tasks-simple --no-color`;
        try {
            let { stdout, stderr } = await exec(commandLine, { cwd: rootPath });
            if (stderr && stderr.length > 0) {
                // Filter out "No license field"
                const errors = stderr.split('\n');
                errors.pop(); // The last line is empty.
                if (!errors.every(value => value.indexOf('No license field') >= 0)) {
                    getOutputChannel().appendLine(stderr);
                    showError();
                }
            }
            let result = [];
            if (stdout) {
                let lines = stdout.split(/\r{0,1}\n/);
                for (let line of lines) {
                    if (line.length === 0) {
                        continue;
                    }
                    let kind = {
                        type: 'gulp',
                        task: line
                    };
                    let options = { cwd: this.workspaceFolder.uri.fsPath };
                    let task = new vscode.Task(kind, this.workspaceFolder, line, 'gulp', new vscode.ShellExecution(await this._gulpCommand, [line], options));
                    result.push(task);
                    let lowerCaseLine = line.toLowerCase();
                    if (isBuildTask(lowerCaseLine)) {
                        task.group = vscode.TaskGroup.Build;
                    }
                    else if (isTestTask(lowerCaseLine)) {
                        task.group = vscode.TaskGroup.Test;
                    }
                }
            }
            return result;
        }
        catch (err) {
            let channel = getOutputChannel();
            if (err.stderr) {
                channel.appendLine(err.stderr);
            }
            if (err.stdout) {
                channel.appendLine(err.stdout);
            }
            channel.appendLine(localize('execFailed', 'Auto detecting gulp for folder {0} failed with error: {1}', this.workspaceFolder.name, err.error ? err.error.toString() : 'unknown'));
            showError();
            return emptyTasks;
        }
    }
    dispose() {
        this.promise = undefined;
        if (this.fileWatcher) {
            this.fileWatcher.dispose();
        }
    }
}
class TaskDetector {
    constructor() {
        this.detectors = new Map();
    }
    start() {
        let folders = vscode.workspace.workspaceFolders;
        if (folders) {
            this.updateWorkspaceFolders(folders, []);
        }
        vscode.workspace.onDidChangeWorkspaceFolders((event) => this.updateWorkspaceFolders(event.added, event.removed));
        vscode.workspace.onDidChangeConfiguration(this.updateConfiguration, this);
    }
    dispose() {
        if (this.taskProvider) {
            this.taskProvider.dispose();
            this.taskProvider = undefined;
        }
        this.detectors.clear();
    }
    updateWorkspaceFolders(added, removed) {
        for (let remove of removed) {
            let detector = this.detectors.get(remove.uri.toString());
            if (detector) {
                detector.dispose();
                this.detectors.delete(remove.uri.toString());
            }
        }
        for (let add of added) {
            let detector = new FolderDetector(add, findGulpCommand(add.uri.fsPath));
            this.detectors.set(add.uri.toString(), detector);
            if (detector.isEnabled()) {
                detector.start();
            }
        }
        this.updateProvider();
    }
    updateConfiguration() {
        for (let detector of this.detectors.values()) {
            detector.dispose();
            this.detectors.delete(detector.workspaceFolder.uri.toString());
        }
        let folders = vscode.workspace.workspaceFolders;
        if (folders) {
            for (let folder of folders) {
                if (!this.detectors.has(folder.uri.toString())) {
                    let detector = new FolderDetector(folder, findGulpCommand(folder.uri.fsPath));
                    this.detectors.set(folder.uri.toString(), detector);
                    if (detector.isEnabled()) {
                        detector.start();
                    }
                }
            }
        }
        this.updateProvider();
    }
    updateProvider() {
        if (!this.taskProvider && this.detectors.size > 0) {
            const thisCapture = this;
            this.taskProvider = vscode.tasks.registerTaskProvider('gulp', {
                provideTasks() {
                    return thisCapture.getTasks();
                },
                resolveTask(_task) {
                    return thisCapture.getTask(_task);
                }
            });
        }
        else if (this.taskProvider && this.detectors.size === 0) {
            this.taskProvider.dispose();
            this.taskProvider = undefined;
        }
    }
    getTasks() {
        return this.computeTasks();
    }
    computeTasks() {
        if (this.detectors.size === 0) {
            return Promise.resolve([]);
        }
        else if (this.detectors.size === 1) {
            return this.detectors.values().next().value.getTasks();
        }
        else {
            let promises = [];
            for (let detector of this.detectors.values()) {
                promises.push(detector.getTasks().then((value) => value, () => []));
            }
            return Promise.all(promises).then((values) => {
                let result = [];
                for (let tasks of values) {
                    if (tasks && tasks.length > 0) {
                        result.push(...tasks);
                    }
                }
                return result;
            });
        }
    }
    async getTask(task) {
        if (this.detectors.size === 0) {
            return undefined;
        }
        else if (this.detectors.size === 1) {
            return this.detectors.values().next().value.getTask(task);
        }
        else {
            if ((task.scope === vscode.TaskScope.Workspace) || (task.scope === vscode.TaskScope.Global)) {
                // Not supported, we don't have enough info to create the task.
                return undefined;
            }
            else if (task.scope) {
                const detector = this.detectors.get(task.scope.uri.toString());
                if (detector) {
                    return detector.getTask(task);
                }
            }
            return undefined;
        }
    }
}
let detector;
function activate(_context) {
    detector = new TaskDetector();
    detector.start();
}
exports.activate = activate;
function deactivate() {
    detector.dispose();
}
exports.deactivate = deactivate;
//# sourceMappingURL=main.js.map