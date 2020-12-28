"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.getScripts = exports.findScriptAtPosition = exports.findAllScriptRanges = exports.startDebugging = exports.runScript = exports.hasPackageJson = exports.getPackageJsonUriFromTask = exports.createTask = exports.getTaskName = exports.isAutoDetectionEnabled = exports.provideNpmScripts = exports.detectNpmScriptsForFolder = exports.hasNpmScripts = exports.getPackageManager = exports.isWorkspaceFolder = exports.invalidateTasksCache = exports.NpmTaskProvider = void 0;
const vscode_1 = require("vscode");
const path = require("path");
const fs = require("fs");
const minimatch = require("minimatch");
const nls = require("vscode-nls");
const jsonc_parser_1 = require("jsonc-parser");
const preferred_pm_1 = require("./preferred-pm");
const localize = nls.loadMessageBundle();
let cachedTasks = undefined;
const INSTALL_SCRIPT = 'install';
class NpmTaskProvider {
    constructor(context) {
        this.context = context;
    }
    get tasksWithLocation() {
        return provideNpmScripts(this.context);
    }
    async provideTasks() {
        const tasks = await provideNpmScripts(this.context);
        return tasks.map(task => task.task);
    }
    resolveTask(_task) {
        const npmTask = _task.definition.script;
        if (npmTask) {
            const kind = _task.definition;
            let packageJsonUri;
            if (_task.scope === undefined || _task.scope === vscode_1.TaskScope.Global || _task.scope === vscode_1.TaskScope.Workspace) {
                // scope is required to be a WorkspaceFolder for resolveTask
                return undefined;
            }
            if (kind.path) {
                packageJsonUri = _task.scope.uri.with({ path: _task.scope.uri.path + '/' + kind.path + 'package.json' });
            }
            else {
                packageJsonUri = _task.scope.uri.with({ path: _task.scope.uri.path + '/package.json' });
            }
            return createTask(this.context, kind, `${kind.script === INSTALL_SCRIPT ? '' : 'run '}${kind.script}`, _task.scope, packageJsonUri);
        }
        return undefined;
    }
}
exports.NpmTaskProvider = NpmTaskProvider;
function invalidateTasksCache() {
    cachedTasks = undefined;
}
exports.invalidateTasksCache = invalidateTasksCache;
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
        if (name === testName) {
            return true;
        }
    }
    return false;
}
function getPrePostScripts(scripts) {
    const prePostScripts = new Set([
        'preuninstall', 'postuninstall', 'prepack', 'postpack', 'preinstall', 'postinstall',
        'prepack', 'postpack', 'prepublish', 'postpublish', 'preversion', 'postversion',
        'prestop', 'poststop', 'prerestart', 'postrestart', 'preshrinkwrap', 'postshrinkwrap',
        'pretest', 'postest', 'prepublishOnly'
    ]);
    let keys = Object.keys(scripts);
    for (const script of keys) {
        const prepost = ['pre' + script, 'post' + script];
        prepost.forEach(each => {
            if (scripts[each] !== undefined) {
                prePostScripts.add(each);
            }
        });
    }
    return prePostScripts;
}
function isWorkspaceFolder(value) {
    return value && typeof value !== 'number';
}
exports.isWorkspaceFolder = isWorkspaceFolder;
async function getPackageManager(extensionContext, folder) {
    let packageManagerName = vscode_1.workspace.getConfiguration('npm', folder).get('packageManager', 'npm');
    if (packageManagerName === 'auto') {
        const { name, multiplePMDetected } = await preferred_pm_1.findPreferredPM(folder.fsPath);
        packageManagerName = name;
        const neverShowWarning = 'npm.multiplePMWarning.neverShow';
        if (multiplePMDetected && !extensionContext.globalState.get(neverShowWarning)) {
            const multiplePMWarning = localize('npm.multiplePMWarning', 'Using {0} as the preferred package manager. Found multiple lockfiles for {1}.', packageManagerName, folder.fsPath);
            const neverShowAgain = localize('npm.multiplePMWarning.doNotShow', "Do not show again");
            const learnMore = localize('npm.multiplePMWarning.learnMore', "Learn more");
            vscode_1.window.showInformationMessage(multiplePMWarning, learnMore, neverShowAgain).then(result => {
                switch (result) {
                    case neverShowAgain:
                        extensionContext.globalState.update(neverShowWarning, true);
                        break;
                    case learnMore: vscode_1.env.openExternal(vscode_1.Uri.parse('https://nodejs.dev/learn/the-package-lock-json-file'));
                }
            });
        }
    }
    return packageManagerName;
}
exports.getPackageManager = getPackageManager;
async function hasNpmScripts() {
    let folders = vscode_1.workspace.workspaceFolders;
    if (!folders) {
        return false;
    }
    try {
        for (const folder of folders) {
            if (isAutoDetectionEnabled(folder)) {
                let relativePattern = new vscode_1.RelativePattern(folder, '**/package.json');
                let paths = await vscode_1.workspace.findFiles(relativePattern, '**/node_modules/**');
                if (paths.length > 0) {
                    return true;
                }
            }
        }
        return false;
    }
    catch (error) {
        return Promise.reject(error);
    }
}
exports.hasNpmScripts = hasNpmScripts;
async function detectNpmScripts(context) {
    let emptyTasks = [];
    let allTasks = [];
    let visitedPackageJsonFiles = new Set();
    let folders = vscode_1.workspace.workspaceFolders;
    if (!folders) {
        return emptyTasks;
    }
    try {
        for (const folder of folders) {
            if (isAutoDetectionEnabled(folder)) {
                let relativePattern = new vscode_1.RelativePattern(folder, '**/package.json');
                let paths = await vscode_1.workspace.findFiles(relativePattern, '**/{node_modules,.vscode-test}/**');
                for (const path of paths) {
                    if (!isExcluded(folder, path) && !visitedPackageJsonFiles.has(path.fsPath)) {
                        let tasks = await provideNpmScriptsForFolder(context, path);
                        visitedPackageJsonFiles.add(path.fsPath);
                        allTasks.push(...tasks);
                    }
                }
            }
        }
        return allTasks;
    }
    catch (error) {
        return Promise.reject(error);
    }
}
async function detectNpmScriptsForFolder(context, folder) {
    let folderTasks = [];
    try {
        let relativePattern = new vscode_1.RelativePattern(folder.fsPath, '**/package.json');
        let paths = await vscode_1.workspace.findFiles(relativePattern, '**/node_modules/**');
        let visitedPackageJsonFiles = new Set();
        for (const path of paths) {
            if (!visitedPackageJsonFiles.has(path.fsPath)) {
                let tasks = await provideNpmScriptsForFolder(context, path);
                visitedPackageJsonFiles.add(path.fsPath);
                folderTasks.push(...tasks.map(t => ({ label: t.task.name, task: t.task })));
            }
        }
        return folderTasks;
    }
    catch (error) {
        return Promise.reject(error);
    }
}
exports.detectNpmScriptsForFolder = detectNpmScriptsForFolder;
async function provideNpmScripts(context) {
    if (!cachedTasks) {
        cachedTasks = await detectNpmScripts(context);
    }
    return cachedTasks;
}
exports.provideNpmScripts = provideNpmScripts;
function isAutoDetectionEnabled(folder) {
    return vscode_1.workspace.getConfiguration('npm', folder === null || folder === void 0 ? void 0 : folder.uri).get('autoDetect') === 'on';
}
exports.isAutoDetectionEnabled = isAutoDetectionEnabled;
function isExcluded(folder, packageJsonUri) {
    function testForExclusionPattern(path, pattern) {
        return minimatch(path, pattern, { dot: true });
    }
    let exclude = vscode_1.workspace.getConfiguration('npm', folder.uri).get('exclude');
    let packageJsonFolder = path.dirname(packageJsonUri.fsPath);
    if (exclude) {
        if (Array.isArray(exclude)) {
            for (let pattern of exclude) {
                if (testForExclusionPattern(packageJsonFolder, pattern)) {
                    return true;
                }
            }
        }
        else if (testForExclusionPattern(packageJsonFolder, exclude)) {
            return true;
        }
    }
    return false;
}
function isDebugScript(script) {
    let match = script.match(/--(inspect|debug)(-brk)?(=((\[[0-9a-fA-F:]*\]|[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+|[a-zA-Z0-9\.]*):)?(\d+))?/);
    return match !== null;
}
async function provideNpmScriptsForFolder(context, packageJsonUri) {
    let emptyTasks = [];
    let folder = vscode_1.workspace.getWorkspaceFolder(packageJsonUri);
    if (!folder) {
        return emptyTasks;
    }
    let scripts = await getScripts(packageJsonUri);
    if (!scripts) {
        return emptyTasks;
    }
    const result = [];
    const prePostScripts = getPrePostScripts(scripts);
    for (const each of scripts.keys()) {
        const scriptValue = scripts.get(each);
        const task = await createTask(context, each, `run ${each}`, folder, packageJsonUri, scriptValue.script);
        const lowerCaseTaskName = each.toLowerCase();
        if (isBuildTask(lowerCaseTaskName)) {
            task.group = vscode_1.TaskGroup.Build;
        }
        else if (isTestTask(lowerCaseTaskName)) {
            task.group = vscode_1.TaskGroup.Test;
        }
        if (prePostScripts.has(each)) {
            task.group = vscode_1.TaskGroup.Clean; // hack: use Clean group to tag pre/post scripts
        }
        // todo@connor4312: all scripts are now debuggable, what is a 'debug script'?
        if (isDebugScript(scriptValue.script)) {
            task.group = vscode_1.TaskGroup.Rebuild; // hack: use Rebuild group to tag debug scripts
        }
        result.push({ task, location: scriptValue.location });
    }
    // always add npm install (without a problem matcher)
    result.push({ task: await createTask(context, INSTALL_SCRIPT, INSTALL_SCRIPT, folder, packageJsonUri, 'install dependencies from package', []) });
    return result;
}
function getTaskName(script, relativePath) {
    if (relativePath && relativePath.length) {
        return `${script} - ${relativePath.substring(0, relativePath.length - 1)}`;
    }
    return script;
}
exports.getTaskName = getTaskName;
async function createTask(context, script, cmd, folder, packageJsonUri, detail, matcher) {
    let kind;
    if (typeof script === 'string') {
        kind = { type: 'npm', script: script };
    }
    else {
        kind = script;
    }
    const packageManager = await getPackageManager(context, folder.uri);
    async function getCommandLine(cmd) {
        if (vscode_1.workspace.getConfiguration('npm', folder.uri).get('runSilent')) {
            return `${packageManager} --silent ${cmd}`;
        }
        return `${packageManager} ${cmd}`;
    }
    function getRelativePath(packageJsonUri) {
        let rootUri = folder.uri;
        let absolutePath = packageJsonUri.path.substring(0, packageJsonUri.path.length - 'package.json'.length);
        return absolutePath.substring(rootUri.path.length + 1);
    }
    let relativePackageJson = getRelativePath(packageJsonUri);
    if (relativePackageJson.length) {
        kind.path = relativePackageJson;
    }
    let taskName = getTaskName(kind.script, relativePackageJson);
    let cwd = path.dirname(packageJsonUri.fsPath);
    const task = new vscode_1.Task(kind, folder, taskName, 'npm', new vscode_1.ShellExecution(await getCommandLine(cmd), { cwd: cwd }), matcher);
    task.detail = detail;
    return task;
}
exports.createTask = createTask;
function getPackageJsonUriFromTask(task) {
    if (isWorkspaceFolder(task.scope)) {
        if (task.definition.path) {
            return vscode_1.Uri.file(path.join(task.scope.uri.fsPath, task.definition.path, 'package.json'));
        }
        else {
            return vscode_1.Uri.file(path.join(task.scope.uri.fsPath, 'package.json'));
        }
    }
    return null;
}
exports.getPackageJsonUriFromTask = getPackageJsonUriFromTask;
async function hasPackageJson() {
    let folders = vscode_1.workspace.workspaceFolders;
    if (!folders) {
        return false;
    }
    for (const folder of folders) {
        if (folder.uri.scheme === 'file') {
            let packageJson = path.join(folder.uri.fsPath, 'package.json');
            if (await exists(packageJson)) {
                return true;
            }
        }
    }
    return false;
}
exports.hasPackageJson = hasPackageJson;
async function exists(file) {
    return new Promise((resolve, _reject) => {
        fs.exists(file, (value) => {
            resolve(value);
        });
    });
}
async function runScript(context, script, document) {
    let uri = document.uri;
    let folder = vscode_1.workspace.getWorkspaceFolder(uri);
    if (folder) {
        let task = await createTask(context, script, `run ${script}`, folder, uri);
        vscode_1.tasks.executeTask(task);
    }
}
exports.runScript = runScript;
async function startDebugging(context, scriptName, cwd, folder) {
    const config = {
        type: 'pwa-node',
        request: 'launch',
        name: `Debug ${scriptName}`,
        cwd,
        runtimeExecutable: await getPackageManager(context, folder.uri),
        runtimeArgs: [
            'run',
            scriptName,
        ],
    };
    if (folder) {
        vscode_1.debug.startDebugging(folder, config);
    }
}
exports.startDebugging = startDebugging;
async function findAllScripts(document, buffer) {
    let scripts = new Map();
    let script = undefined;
    let inScripts = false;
    let scriptOffset = 0;
    let visitor = {
        onError(_error, _offset, _length) {
            console.log(_error);
        },
        onObjectEnd() {
            if (inScripts) {
                inScripts = false;
            }
        },
        onLiteralValue(value, _offset, _length) {
            if (script) {
                if (typeof value === 'string') {
                    scripts.set(script, { script: value, location: { document: document.uri, line: document.positionAt(scriptOffset) } });
                }
                script = undefined;
            }
        },
        onObjectProperty(property, offset, _length) {
            if (property === 'scripts') {
                inScripts = true;
            }
            else if (inScripts && !script) {
                script = property;
                scriptOffset = offset;
            }
            else { // nested object which is invalid, ignore the script
                script = undefined;
            }
        }
    };
    jsonc_parser_1.visit(buffer, visitor);
    return scripts;
}
function findAllScriptRanges(buffer) {
    let scripts = new Map();
    let script = undefined;
    let offset;
    let length;
    let inScripts = false;
    let visitor = {
        onError(_error, _offset, _length) {
        },
        onObjectEnd() {
            if (inScripts) {
                inScripts = false;
            }
        },
        onLiteralValue(value, _offset, _length) {
            if (script) {
                scripts.set(script, [offset, length, value]);
                script = undefined;
            }
        },
        onObjectProperty(property, off, len) {
            if (property === 'scripts') {
                inScripts = true;
            }
            else if (inScripts) {
                script = property;
                offset = off;
                length = len;
            }
        }
    };
    jsonc_parser_1.visit(buffer, visitor);
    return scripts;
}
exports.findAllScriptRanges = findAllScriptRanges;
function findScriptAtPosition(buffer, offset) {
    let script = undefined;
    let foundScript = undefined;
    let inScripts = false;
    let scriptStart;
    let visitor = {
        onError(_error, _offset, _length) {
        },
        onObjectEnd() {
            if (inScripts) {
                inScripts = false;
                scriptStart = undefined;
            }
        },
        onLiteralValue(value, nodeOffset, nodeLength) {
            if (inScripts && scriptStart) {
                if (typeof value === 'string' && offset >= scriptStart && offset < nodeOffset + nodeLength) {
                    // found the script
                    inScripts = false;
                    foundScript = script;
                }
                else {
                    script = undefined;
                }
            }
        },
        onObjectProperty(property, nodeOffset) {
            if (property === 'scripts') {
                inScripts = true;
            }
            else if (inScripts) {
                scriptStart = nodeOffset;
                script = property;
            }
            else { // nested object which is invalid, ignore the script
                script = undefined;
            }
        }
    };
    jsonc_parser_1.visit(buffer, visitor);
    return foundScript;
}
exports.findScriptAtPosition = findScriptAtPosition;
async function getScripts(packageJsonUri) {
    if (packageJsonUri.scheme !== 'file') {
        return undefined;
    }
    let packageJson = packageJsonUri.fsPath;
    if (!await exists(packageJson)) {
        return undefined;
    }
    try {
        const document = await vscode_1.workspace.openTextDocument(packageJsonUri);
        let contents = document.getText();
        let json = findAllScripts(document, contents); //JSON.parse(contents);
        return json;
    }
    catch (e) {
        let localizedParseError = localize('npm.parseError', 'Npm task detection: failed to parse the file {0}', packageJsonUri.fsPath);
        throw new Error(localizedParseError);
    }
}
exports.getScripts = getScripts;
//# sourceMappingURL=tasks.js.map