"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const fs = require("fs");
const path = require("path");
const os = require("os");
const cp = require("child_process");
const which = require("which");
const events_1 = require("events");
const iconv = require("iconv-lite");
const filetype = require("file-type");
const util_1 = require("./util");
const vscode_1 = require("vscode");
const encoding_1 = require("./encoding");
const readfile = util_1.denodeify(fs.readFile);
function parseVersion(raw) {
    return raw.replace(/^git version /, '');
}
function findSpecificGit(path, onLookup) {
    return new Promise((c, e) => {
        onLookup(path);
        const buffers = [];
        const child = cp.spawn(path, ['--version']);
        child.stdout.on('data', (b) => buffers.push(b));
        child.on('error', cpErrorHandler(e));
        child.on('exit', code => code ? e(new Error('Not found')) : c({ path, version: parseVersion(Buffer.concat(buffers).toString('utf8').trim()) }));
    });
}
function findGitDarwin(onLookup) {
    return new Promise((c, e) => {
        cp.exec('which git', (err, gitPathBuffer) => {
            if (err) {
                return e('git not found');
            }
            const path = gitPathBuffer.toString().replace(/^\s+|\s+$/g, '');
            function getVersion(path) {
                onLookup(path);
                // make sure git executes
                cp.exec('git --version', (err, stdout) => {
                    if (err) {
                        return e('git not found');
                    }
                    return c({ path, version: parseVersion(stdout.trim()) });
                });
            }
            if (path !== '/usr/bin/git') {
                return getVersion(path);
            }
            // must check if XCode is installed
            cp.exec('xcode-select -p', (err) => {
                if (err && err.code === 2) {
                    // git is not installed, and launching /usr/bin/git
                    // will prompt the user to install it
                    return e('git not found');
                }
                getVersion(path);
            });
        });
    });
}
function findSystemGitWin32(base, onLookup) {
    if (!base) {
        return Promise.reject('Not found');
    }
    return findSpecificGit(path.join(base, 'Git', 'cmd', 'git.exe'), onLookup);
}
function findGitWin32InPath(onLookup) {
    const whichPromise = new Promise((c, e) => which('git.exe', (err, path) => err ? e(err) : c(path)));
    return whichPromise.then(path => findSpecificGit(path, onLookup));
}
function findGitWin32(onLookup) {
    return findSystemGitWin32(process.env['ProgramW6432'], onLookup)
        .then(undefined, () => findSystemGitWin32(process.env['ProgramFiles(x86)'], onLookup))
        .then(undefined, () => findSystemGitWin32(process.env['ProgramFiles'], onLookup))
        .then(undefined, () => findSystemGitWin32(path.join(process.env['LocalAppData'], 'Programs'), onLookup))
        .then(undefined, () => findGitWin32InPath(onLookup));
}
function findGit(hint, onLookup) {
    const first = hint ? findSpecificGit(hint, onLookup) : Promise.reject(null);
    return first
        .then(undefined, () => {
        switch (process.platform) {
            case 'darwin': return findGitDarwin(onLookup);
            case 'win32': return findGitWin32(onLookup);
            default: return findSpecificGit('git', onLookup);
        }
    })
        .then(null, () => Promise.reject(new Error('Git installation not found.')));
}
exports.findGit = findGit;
function cpErrorHandler(cb) {
    return err => {
        if (/ENOENT/.test(err.message)) {
            err = new GitError({
                error: err,
                message: 'Failed to execute git (ENOENT)',
                gitErrorCode: "NotAGitRepository" /* NotAGitRepository */
            });
        }
        cb(err);
    };
}
async function exec(child, cancellationToken) {
    if (!child.stdout || !child.stderr) {
        throw new GitError({ message: 'Failed to get stdout or stderr from git process.' });
    }
    if (cancellationToken && cancellationToken.isCancellationRequested) {
        throw new GitError({ message: 'Cancelled' });
    }
    const disposables = [];
    const once = (ee, name, fn) => {
        ee.once(name, fn);
        disposables.push(util_1.toDisposable(() => ee.removeListener(name, fn)));
    };
    const on = (ee, name, fn) => {
        ee.on(name, fn);
        disposables.push(util_1.toDisposable(() => ee.removeListener(name, fn)));
    };
    let result = Promise.all([
        new Promise((c, e) => {
            once(child, 'error', cpErrorHandler(e));
            once(child, 'exit', c);
        }),
        new Promise(c => {
            const buffers = [];
            on(child.stdout, 'data', (b) => buffers.push(b));
            once(child.stdout, 'close', () => c(Buffer.concat(buffers)));
        }),
        new Promise(c => {
            const buffers = [];
            on(child.stderr, 'data', (b) => buffers.push(b));
            once(child.stderr, 'close', () => c(Buffer.concat(buffers).toString('utf8')));
        })
    ]);
    if (cancellationToken) {
        const cancellationPromise = new Promise((_, e) => {
            util_1.onceEvent(cancellationToken.onCancellationRequested)(() => {
                try {
                    child.kill();
                }
                catch (err) {
                    // noop
                }
                e(new GitError({ message: 'Cancelled' }));
            });
        });
        result = Promise.race([result, cancellationPromise]);
    }
    try {
        const [exitCode, stdout, stderr] = await result;
        return { exitCode, stdout, stderr };
    }
    finally {
        util_1.dispose(disposables);
    }
}
class GitError {
    constructor(data) {
        if (data.error) {
            this.error = data.error;
            this.message = data.error.message;
        }
        else {
            this.error = undefined;
            this.message = '';
        }
        this.message = this.message || data.message || 'Git error';
        this.stdout = data.stdout;
        this.stderr = data.stderr;
        this.exitCode = data.exitCode;
        this.gitErrorCode = data.gitErrorCode;
        this.gitCommand = data.gitCommand;
    }
    toString() {
        let result = this.message + ' ' + JSON.stringify({
            exitCode: this.exitCode,
            gitErrorCode: this.gitErrorCode,
            gitCommand: this.gitCommand,
            stdout: this.stdout,
            stderr: this.stderr
        }, null, 2);
        if (this.error) {
            result += this.error.stack;
        }
        return result;
    }
}
exports.GitError = GitError;
function getGitErrorCode(stderr) {
    if (/Another git process seems to be running in this repository|If no other git process is currently running/.test(stderr)) {
        return "RepositoryIsLocked" /* RepositoryIsLocked */;
    }
    else if (/Authentication failed/.test(stderr)) {
        return "AuthenticationFailed" /* AuthenticationFailed */;
    }
    else if (/Not a git repository/i.test(stderr)) {
        return "NotAGitRepository" /* NotAGitRepository */;
    }
    else if (/bad config file/.test(stderr)) {
        return "BadConfigFile" /* BadConfigFile */;
    }
    else if (/cannot make pipe for command substitution|cannot create standard input pipe/.test(stderr)) {
        return "CantCreatePipe" /* CantCreatePipe */;
    }
    else if (/Repository not found/.test(stderr)) {
        return "RepositoryNotFound" /* RepositoryNotFound */;
    }
    else if (/unable to access/.test(stderr)) {
        return "CantAccessRemote" /* CantAccessRemote */;
    }
    else if (/branch '.+' is not fully merged/.test(stderr)) {
        return "BranchNotFullyMerged" /* BranchNotFullyMerged */;
    }
    else if (/Couldn\'t find remote ref/.test(stderr)) {
        return "NoRemoteReference" /* NoRemoteReference */;
    }
    else if (/A branch named '.+' already exists/.test(stderr)) {
        return "BranchAlreadyExists" /* BranchAlreadyExists */;
    }
    else if (/'.+' is not a valid branch name/.test(stderr)) {
        return "InvalidBranchName" /* InvalidBranchName */;
    }
    return undefined;
}
const COMMIT_FORMAT = '%H\n%ae\n%P\n%B';
class Git {
    constructor(options) {
        this._onOutput = new events_1.EventEmitter();
        this.path = options.gitPath;
        this.env = options.env || {};
    }
    get onOutput() { return this._onOutput; }
    open(repository) {
        return new Repository(this, repository);
    }
    async init(repository) {
        await this.exec(repository, ['init']);
        return;
    }
    async clone(url, parentPath, cancellationToken) {
        let baseFolderName = decodeURI(url).replace(/^.*\//, '').replace(/\.git$/, '') || 'repository';
        let folderName = baseFolderName;
        let folderPath = path.join(parentPath, folderName);
        let count = 1;
        while (count < 20 && await new Promise(c => fs.exists(folderPath, c))) {
            folderName = `${baseFolderName}-${count++}`;
            folderPath = path.join(parentPath, folderName);
        }
        await util_1.mkdirp(parentPath);
        try {
            await this.exec(parentPath, ['clone', url.includes(' ') ? encodeURI(url) : url, folderPath], { cancellationToken });
        }
        catch (err) {
            if (err.stderr) {
                err.stderr = err.stderr.replace(/^Cloning.+$/m, '').trim();
                err.stderr = err.stderr.replace(/^ERROR:\s+/, '').trim();
            }
            throw err;
        }
        return folderPath;
    }
    async getRepositoryRoot(repositoryPath) {
        const result = await this.exec(repositoryPath, ['rev-parse', '--show-toplevel']);
        return path.normalize(result.stdout.trim());
    }
    async exec(cwd, args, options = {}) {
        options = util_1.assign({ cwd }, options || {});
        return await this._exec(args, options);
    }
    async exec2(args, options = {}) {
        return await this._exec(args, options);
    }
    stream(cwd, args, options = {}) {
        options = util_1.assign({ cwd }, options || {});
        return this.spawn(args, options);
    }
    async _exec(args, options = {}) {
        const child = this.spawn(args, options);
        if (options.input) {
            child.stdin.end(options.input, 'utf8');
        }
        const bufferResult = await exec(child, options.cancellationToken);
        if (options.log !== false && bufferResult.stderr.length > 0) {
            this.log(`${bufferResult.stderr}\n`);
        }
        let encoding = options.encoding || 'utf8';
        encoding = iconv.encodingExists(encoding) ? encoding : 'utf8';
        const result = {
            exitCode: bufferResult.exitCode,
            stdout: iconv.decode(bufferResult.stdout, encoding),
            stderr: bufferResult.stderr
        };
        if (bufferResult.exitCode) {
            return Promise.reject(new GitError({
                message: 'Failed to execute git',
                stdout: result.stdout,
                stderr: result.stderr,
                exitCode: result.exitCode,
                gitErrorCode: getGitErrorCode(result.stderr),
                gitCommand: args[0]
            }));
        }
        return result;
    }
    spawn(args, options = {}) {
        if (!this.path) {
            throw new Error('git could not be found in the system.');
        }
        if (!options) {
            options = {};
        }
        if (!options.stdio && !options.input) {
            options.stdio = ['ignore', null, null]; // Unless provided, ignore stdin and leave default streams for stdout and stderr
        }
        options.env = util_1.assign({}, process.env, this.env, options.env || {}, {
            VSCODE_GIT_COMMAND: args[0],
            LC_ALL: 'en_US.UTF-8',
            LANG: 'en_US.UTF-8'
        });
        if (options.log !== false) {
            this.log(`> git ${args.join(' ')}\n`);
        }
        return cp.spawn(this.path, args, options);
    }
    log(output) {
        this._onOutput.emit('log', output);
    }
}
exports.Git = Git;
class GitStatusParser {
    constructor() {
        this.lastRaw = '';
        this.result = [];
    }
    get status() {
        return this.result;
    }
    update(raw) {
        let i = 0;
        let nextI;
        raw = this.lastRaw + raw;
        while ((nextI = this.parseEntry(raw, i)) !== undefined) {
            i = nextI;
        }
        this.lastRaw = raw.substr(i);
    }
    parseEntry(raw, i) {
        if (i + 4 >= raw.length) {
            return;
        }
        let lastIndex;
        const entry = {
            x: raw.charAt(i++),
            y: raw.charAt(i++),
            rename: undefined,
            path: ''
        };
        // space
        i++;
        if (entry.x === 'R' || entry.x === 'C') {
            lastIndex = raw.indexOf('\0', i);
            if (lastIndex === -1) {
                return;
            }
            entry.rename = raw.substring(i, lastIndex);
            i = lastIndex + 1;
        }
        lastIndex = raw.indexOf('\0', i);
        if (lastIndex === -1) {
            return;
        }
        entry.path = raw.substring(i, lastIndex);
        // If path ends with slash, it must be a nested git repo
        if (entry.path[entry.path.length - 1] !== '/') {
            this.result.push(entry);
        }
        return lastIndex + 1;
    }
}
exports.GitStatusParser = GitStatusParser;
function parseGitmodules(raw) {
    const regex = /\r?\n/g;
    let position = 0;
    let match = null;
    const result = [];
    let submodule = {};
    function parseLine(line) {
        const sectionMatch = /^\s*\[submodule "([^"]+)"\]\s*$/.exec(line);
        if (sectionMatch) {
            if (submodule.name && submodule.path && submodule.url) {
                result.push(submodule);
            }
            const name = sectionMatch[1];
            if (name) {
                submodule = { name };
                return;
            }
        }
        if (!submodule) {
            return;
        }
        const propertyMatch = /^\s*(\w+) = (.*)$/.exec(line);
        if (!propertyMatch) {
            return;
        }
        const [, key, value] = propertyMatch;
        switch (key) {
            case 'path':
                submodule.path = value;
                break;
            case 'url':
                submodule.url = value;
                break;
        }
    }
    while (match = regex.exec(raw)) {
        parseLine(raw.substring(position, match.index));
        position = match.index + match[0].length;
    }
    parseLine(raw.substring(position));
    if (submodule.name && submodule.path && submodule.url) {
        result.push(submodule);
    }
    return result;
}
exports.parseGitmodules = parseGitmodules;
function parseGitCommit(raw) {
    const match = /^([0-9a-f]{40})\n(.*)\n(.*)\n([^]*)$/m.exec(raw.trim());
    if (!match) {
        return null;
    }
    const parents = match[3] ? match[3].split(' ') : [];
    return { hash: match[1], message: match[4], parents, authorEmail: match[2] };
}
exports.parseGitCommit = parseGitCommit;
function parseLsTree(raw) {
    return raw.split('\n')
        .filter(l => !!l)
        .map(line => /^(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(.*)$/.exec(line))
        .filter(m => !!m)
        .map(([, mode, type, object, size, file]) => ({ mode, type, object, size, file }));
}
exports.parseLsTree = parseLsTree;
function parseLsFiles(raw) {
    return raw.split('\n')
        .filter(l => !!l)
        .map(line => /^(\S+)\s+(\S+)\s+(\S+)\s+(.*)$/.exec(line))
        .filter(m => !!m)
        .map(([, mode, object, stage, file]) => ({ mode, object, stage, file }));
}
exports.parseLsFiles = parseLsFiles;
var ForcePushMode;
(function (ForcePushMode) {
    ForcePushMode[ForcePushMode["Force"] = 0] = "Force";
    ForcePushMode[ForcePushMode["ForceWithLease"] = 1] = "ForceWithLease";
})(ForcePushMode = exports.ForcePushMode || (exports.ForcePushMode = {}));
class Repository {
    constructor(_git, repositoryRoot) {
        this._git = _git;
        this.repositoryRoot = repositoryRoot;
    }
    get git() {
        return this._git;
    }
    get root() {
        return this.repositoryRoot;
    }
    // TODO@Joao: rename to exec
    async run(args, options = {}) {
        return await this.git.exec(this.repositoryRoot, args, options);
    }
    stream(args, options = {}) {
        return this.git.stream(this.repositoryRoot, args, options);
    }
    spawn(args, options = {}) {
        return this.git.spawn(args, options);
    }
    async config(scope, key, value = null, options = {}) {
        const args = ['config'];
        if (scope) {
            args.push('--' + scope);
        }
        args.push(key);
        if (value) {
            args.push(value);
        }
        const result = await this.run(args, options);
        return result.stdout.trim();
    }
    async getConfigs(scope) {
        const args = ['config'];
        if (scope) {
            args.push('--' + scope);
        }
        args.push('-l');
        const result = await this.run(args);
        const lines = result.stdout.trim().split(/\r|\r\n|\n/);
        return lines.map(entry => {
            const equalsIndex = entry.indexOf('=');
            return { key: entry.substr(0, equalsIndex), value: entry.substr(equalsIndex + 1) };
        });
    }
    async log(options) {
        const maxEntries = options && typeof options.maxEntries === 'number' && options.maxEntries > 0 ? options.maxEntries : 32;
        const args = ['log', '-' + maxEntries, `--pretty=format:${COMMIT_FORMAT}%x00%x00`];
        const gitResult = await this.run(args);
        if (gitResult.exitCode) {
            // An empty repo.
            return [];
        }
        const s = gitResult.stdout;
        const result = [];
        let index = 0;
        while (index < s.length) {
            let nextIndex = s.indexOf('\x00\x00', index);
            if (nextIndex === -1) {
                nextIndex = s.length;
            }
            let entry = s.substr(index, nextIndex - index);
            if (entry.startsWith('\n')) {
                entry = entry.substring(1);
            }
            const commit = parseGitCommit(entry);
            if (!commit) {
                break;
            }
            result.push(commit);
            index = nextIndex + 2;
        }
        return result;
    }
    async bufferString(object, encoding = 'utf8', autoGuessEncoding = false) {
        const stdout = await this.buffer(object);
        if (autoGuessEncoding) {
            encoding = encoding_1.detectEncoding(stdout) || encoding;
        }
        encoding = iconv.encodingExists(encoding) ? encoding : 'utf8';
        return iconv.decode(stdout, encoding);
    }
    async buffer(object) {
        const child = this.stream(['show', object]);
        if (!child.stdout) {
            return Promise.reject('Can\'t open file from git');
        }
        const { exitCode, stdout, stderr } = await exec(child);
        if (exitCode) {
            const err = new GitError({
                message: 'Could not show object.',
                exitCode
            });
            if (/exists on disk, but not in/.test(stderr)) {
                err.gitErrorCode = "WrongCase" /* WrongCase */;
            }
            return Promise.reject(err);
        }
        return stdout;
    }
    async getObjectDetails(treeish, path) {
        if (!treeish) { // index
            const elements = await this.lsfiles(path);
            if (elements.length === 0) {
                throw new GitError({ message: 'Error running ls-files' });
            }
            const { mode, object } = elements[0];
            const catFile = await this.run(['cat-file', '-s', object]);
            const size = parseInt(catFile.stdout);
            return { mode, object, size };
        }
        const elements = await this.lstree(treeish, path);
        if (elements.length === 0) {
            throw new GitError({ message: 'Error running ls-files' });
        }
        const { mode, object, size } = elements[0];
        return { mode, object, size: parseInt(size) };
    }
    async lstree(treeish, path) {
        const { stdout } = await this.run(['ls-tree', '-l', treeish, '--', path]);
        return parseLsTree(stdout);
    }
    async lsfiles(path) {
        const { stdout } = await this.run(['ls-files', '--stage', '--', path]);
        return parseLsFiles(stdout);
    }
    async getGitRelativePath(ref, relativePath) {
        const relativePathLowercase = relativePath.toLowerCase();
        const dirname = path.posix.dirname(relativePath) + '/';
        const elements = ref ? await this.lstree(ref, dirname) : await this.lsfiles(dirname);
        const element = elements.filter(file => file.file.toLowerCase() === relativePathLowercase)[0];
        if (!element) {
            throw new GitError({ message: 'Git relative path not found.' });
        }
        return element.file;
    }
    async detectObjectType(object) {
        const child = await this.stream(['show', object]);
        const buffer = await util_1.readBytes(child.stdout, 4100);
        try {
            child.kill();
        }
        catch (err) {
            // noop
        }
        const encoding = util_1.detectUnicodeEncoding(buffer);
        let isText = true;
        if (encoding !== "utf16be" /* UTF16be */ && encoding !== "utf16le" /* UTF16le */) {
            for (let i = 0; i < buffer.length; i++) {
                if (buffer.readInt8(i) === 0) {
                    isText = false;
                    break;
                }
            }
        }
        if (!isText) {
            const result = filetype(buffer);
            if (!result) {
                return { mimetype: 'application/octet-stream' };
            }
            else {
                return { mimetype: result.mime };
            }
        }
        if (encoding) {
            return { mimetype: 'text/plain', encoding };
        }
        else {
            // TODO@JOAO: read the setting OUTSIDE!
            return { mimetype: 'text/plain' };
        }
    }
    async apply(patch, reverse) {
        const args = ['apply', patch];
        if (reverse) {
            args.push('-R');
        }
        try {
            await this.run(args);
        }
        catch (err) {
            if (/patch does not apply/.test(err.stderr)) {
                err.gitErrorCode = "PatchDoesNotApply" /* PatchDoesNotApply */;
            }
            throw err;
        }
    }
    async diff(cached = false) {
        const args = ['diff'];
        if (cached) {
            args.push('--cached');
        }
        const result = await this.run(args);
        return result.stdout;
    }
    async diffWithHEAD(path) {
        if (!path) {
            return await this.diffFiles(false);
        }
        const args = ['diff', '--', path];
        const result = await this.run(args);
        return result.stdout;
    }
    async diffWith(ref, path) {
        if (!path) {
            return await this.diffFiles(false, ref);
        }
        const args = ['diff', ref, '--', path];
        const result = await this.run(args);
        return result.stdout;
    }
    async diffIndexWithHEAD(path) {
        if (!path) {
            return await this.diffFiles(true);
        }
        const args = ['diff', '--cached', '--', path];
        const result = await this.run(args);
        return result.stdout;
    }
    async diffIndexWith(ref, path) {
        if (!path) {
            return await this.diffFiles(true, ref);
        }
        const args = ['diff', '--cached', ref, '--', path];
        const result = await this.run(args);
        return result.stdout;
    }
    async diffBlobs(object1, object2) {
        const args = ['diff', object1, object2];
        const result = await this.run(args);
        return result.stdout;
    }
    async diffBetween(ref1, ref2, path) {
        const range = `${ref1}...${ref2}`;
        if (!path) {
            return await this.diffFiles(false, range);
        }
        const args = ['diff', range, '--', path];
        const result = await this.run(args);
        return result.stdout.trim();
    }
    async diffFiles(cached, ref) {
        const args = ['diff', '--name-status', '-z', '--diff-filter=ADMR'];
        if (cached) {
            args.push('--cached');
        }
        if (ref) {
            args.push(ref);
        }
        const gitResult = await this.run(args);
        if (gitResult.exitCode) {
            return [];
        }
        const entries = gitResult.stdout.split('\x00');
        let index = 0;
        const result = [];
        entriesLoop: while (index < entries.length - 1) {
            const change = entries[index++];
            const resourcePath = entries[index++];
            if (!change || !resourcePath) {
                break;
            }
            const originalUri = vscode_1.Uri.file(path.isAbsolute(resourcePath) ? resourcePath : path.join(this.repositoryRoot, resourcePath));
            let status = 7 /* UNTRACKED */;
            // Copy or Rename status comes with a number, e.g. 'R100'. We don't need the number, so we use only first character of the status.
            switch (change[0]) {
                case 'M':
                    status = 5 /* MODIFIED */;
                    break;
                case 'A':
                    status = 1 /* INDEX_ADDED */;
                    break;
                case 'D':
                    status = 6 /* DELETED */;
                    break;
                // Rename contains two paths, the second one is what the file is renamed/copied to.
                case 'R':
                    if (index >= entries.length) {
                        break;
                    }
                    const newPath = entries[index++];
                    if (!newPath) {
                        break;
                    }
                    const uri = vscode_1.Uri.file(path.isAbsolute(newPath) ? newPath : path.join(this.repositoryRoot, newPath));
                    result.push({
                        uri,
                        renameUri: uri,
                        originalUri,
                        status: 3 /* INDEX_RENAMED */
                    });
                    continue;
                default:
                    // Unknown status
                    break entriesLoop;
            }
            result.push({
                status,
                originalUri,
                uri: originalUri,
                renameUri: originalUri,
            });
        }
        return result;
    }
    async getMergeBase(ref1, ref2) {
        const args = ['merge-base', ref1, ref2];
        const result = await this.run(args);
        return result.stdout.trim();
    }
    async hashObject(data) {
        const args = ['hash-object', '-w', '--stdin'];
        const result = await this.run(args, { input: data });
        return result.stdout.trim();
    }
    async add(paths) {
        const args = ['add', '-A', '--'];
        if (paths && paths.length) {
            args.push.apply(args, paths);
        }
        else {
            args.push('.');
        }
        await this.run(args);
    }
    async rm(paths) {
        const args = ['rm', '--'];
        if (!paths || !paths.length) {
            return;
        }
        args.push(...paths);
        await this.run(args);
    }
    async stage(path, data) {
        const child = this.stream(['hash-object', '--stdin', '-w', '--path', path], { stdio: [null, null, null] });
        child.stdin.end(data, 'utf8');
        const { exitCode, stdout } = await exec(child);
        const hash = stdout.toString('utf8');
        if (exitCode) {
            throw new GitError({
                message: 'Could not hash object.',
                exitCode: exitCode
            });
        }
        let mode;
        try {
            const details = await this.getObjectDetails('HEAD', path);
            mode = details.mode;
        }
        catch (err) {
            mode = '100644';
        }
        await this.run(['update-index', '--cacheinfo', mode, hash, path]);
    }
    async checkout(treeish, paths, opts = Object.create(null)) {
        const args = ['checkout', '-q'];
        if (opts.track) {
            args.push('--track');
        }
        if (treeish) {
            args.push(treeish);
        }
        if (paths && paths.length) {
            args.push('--');
            args.push.apply(args, paths);
        }
        try {
            await this.run(args);
        }
        catch (err) {
            if (/Please,? commit your changes or stash them/.test(err.stderr || '')) {
                err.gitErrorCode = "DirtyWorkTree" /* DirtyWorkTree */;
            }
            throw err;
        }
    }
    async commit(message, opts = Object.create(null)) {
        const args = ['commit', '--quiet', '--allow-empty-message', '--file', '-'];
        if (opts.all) {
            args.push('--all');
        }
        if (opts.amend) {
            args.push('--amend');
        }
        if (opts.signoff) {
            args.push('--signoff');
        }
        if (opts.signCommit) {
            args.push('-S');
        }
        if (opts.empty) {
            args.push('--allow-empty');
        }
        try {
            await this.run(args, { input: message || '' });
        }
        catch (commitErr) {
            await this.handleCommitError(commitErr);
        }
    }
    async rebaseContinue() {
        const args = ['rebase', '--continue'];
        try {
            await this.run(args);
        }
        catch (commitErr) {
            await this.handleCommitError(commitErr);
        }
    }
    async handleCommitError(commitErr) {
        if (/not possible because you have unmerged files/.test(commitErr.stderr || '')) {
            commitErr.gitErrorCode = "UnmergedChanges" /* UnmergedChanges */;
            throw commitErr;
        }
        try {
            await this.run(['config', '--get-all', 'user.name']);
        }
        catch (err) {
            err.gitErrorCode = "NoUserNameConfigured" /* NoUserNameConfigured */;
            throw err;
        }
        try {
            await this.run(['config', '--get-all', 'user.email']);
        }
        catch (err) {
            err.gitErrorCode = "NoUserEmailConfigured" /* NoUserEmailConfigured */;
            throw err;
        }
        throw commitErr;
    }
    async branch(name, checkout, ref) {
        const args = checkout ? ['checkout', '-q', '-b', name, '--no-track'] : ['branch', '-q', name];
        if (ref) {
            args.push(ref);
        }
        await this.run(args);
    }
    async deleteBranch(name, force) {
        const args = ['branch', force ? '-D' : '-d', name];
        await this.run(args);
    }
    async renameBranch(name) {
        const args = ['branch', '-m', name];
        await this.run(args);
    }
    async setBranchUpstream(name, upstream) {
        const args = ['branch', '--set-upstream-to', upstream, name];
        await this.run(args);
    }
    async deleteRef(ref) {
        const args = ['update-ref', '-d', ref];
        await this.run(args);
    }
    async merge(ref) {
        const args = ['merge', ref];
        try {
            await this.run(args);
        }
        catch (err) {
            if (/^CONFLICT /m.test(err.stdout || '')) {
                err.gitErrorCode = "Conflict" /* Conflict */;
            }
            throw err;
        }
    }
    async tag(name, message) {
        let args = ['tag'];
        if (message) {
            args = [...args, '-a', name, '-m', message];
        }
        else {
            args = [...args, name];
        }
        await this.run(args);
    }
    async clean(paths) {
        const pathsByGroup = util_1.groupBy(paths, p => path.dirname(p));
        const groups = Object.keys(pathsByGroup).map(k => pathsByGroup[k]);
        const tasks = groups.map(paths => () => this.run(['clean', '-f', '-q', '--'].concat(paths)));
        for (let task of tasks) {
            await task();
        }
    }
    async undo() {
        await this.run(['clean', '-fd']);
        try {
            await this.run(['checkout', '--', '.']);
        }
        catch (err) {
            if (/did not match any file\(s\) known to git\./.test(err.stderr || '')) {
                return;
            }
            throw err;
        }
    }
    async reset(treeish, hard = false) {
        const args = ['reset', hard ? '--hard' : '--soft', treeish];
        await this.run(args);
    }
    async revert(treeish, paths) {
        const result = await this.run(['branch']);
        let args;
        // In case there are no branches, we must use rm --cached
        if (!result.stdout) {
            args = ['rm', '--cached', '-r', '--'];
        }
        else {
            args = ['reset', '-q', treeish, '--'];
        }
        if (paths && paths.length) {
            args.push.apply(args, paths);
        }
        else {
            args.push('.');
        }
        try {
            await this.run(args);
        }
        catch (err) {
            // In case there are merge conflicts to be resolved, git reset will output
            // some "needs merge" data. We try to get around that.
            if (/([^:]+: needs merge\n)+/m.test(err.stdout || '')) {
                return;
            }
            throw err;
        }
    }
    async addRemote(name, url) {
        const args = ['remote', 'add', name, url];
        await this.run(args);
    }
    async removeRemote(name) {
        const args = ['remote', 'rm', name];
        await this.run(args);
    }
    async fetch(options = {}) {
        const args = ['fetch'];
        if (options.remote) {
            args.push(options.remote);
            if (options.ref) {
                args.push(options.ref);
            }
        }
        else if (options.all) {
            args.push('--all');
        }
        if (options.prune) {
            args.push('--prune');
        }
        if (typeof options.depth === 'number') {
            args.push(`--depth=${options.depth}`);
        }
        try {
            await this.run(args);
        }
        catch (err) {
            if (/No remote repository specified\./.test(err.stderr || '')) {
                err.gitErrorCode = "NoRemoteRepositorySpecified" /* NoRemoteRepositorySpecified */;
            }
            else if (/Could not read from remote repository/.test(err.stderr || '')) {
                err.gitErrorCode = "RemoteConnectionError" /* RemoteConnectionError */;
            }
            throw err;
        }
    }
    async pull(rebase, remote, branch, options = {}) {
        const args = ['pull', '--tags'];
        if (options.unshallow) {
            args.push('--unshallow');
        }
        if (rebase) {
            args.push('-r');
        }
        if (remote && branch) {
            args.push(remote);
            args.push(branch);
        }
        try {
            await this.run(args);
        }
        catch (err) {
            if (/^CONFLICT \([^)]+\): \b/m.test(err.stdout || '')) {
                err.gitErrorCode = "Conflict" /* Conflict */;
            }
            else if (/Please tell me who you are\./.test(err.stderr || '')) {
                err.gitErrorCode = "NoUserNameConfigured" /* NoUserNameConfigured */;
            }
            else if (/Could not read from remote repository/.test(err.stderr || '')) {
                err.gitErrorCode = "RemoteConnectionError" /* RemoteConnectionError */;
            }
            else if (/Pull is not possible because you have unmerged files|Cannot pull with rebase: You have unstaged changes|Your local changes to the following files would be overwritten|Please, commit your changes before you can merge/i.test(err.stderr)) {
                err.stderr = err.stderr.replace(/Cannot pull with rebase: You have unstaged changes/i, 'Cannot pull with rebase, you have unstaged changes');
                err.gitErrorCode = "DirtyWorkTree" /* DirtyWorkTree */;
            }
            else if (/cannot lock ref|unable to update local ref/i.test(err.stderr || '')) {
                err.gitErrorCode = "CantLockRef" /* CantLockRef */;
            }
            else if (/cannot rebase onto multiple branches/i.test(err.stderr || '')) {
                err.gitErrorCode = "CantRebaseMultipleBranches" /* CantRebaseMultipleBranches */;
            }
            throw err;
        }
    }
    async push(remote, name, setUpstream = false, tags = false, forcePushMode) {
        const args = ['push'];
        if (forcePushMode === ForcePushMode.ForceWithLease) {
            args.push('--force-with-lease');
        }
        else if (forcePushMode === ForcePushMode.Force) {
            args.push('--force');
        }
        if (setUpstream) {
            args.push('-u');
        }
        if (tags) {
            args.push('--tags');
        }
        if (remote) {
            args.push(remote);
        }
        if (name) {
            args.push(name);
        }
        try {
            await this.run(args);
        }
        catch (err) {
            if (/^error: failed to push some refs to\b/m.test(err.stderr || '')) {
                err.gitErrorCode = "PushRejected" /* PushRejected */;
            }
            else if (/Could not read from remote repository/.test(err.stderr || '')) {
                err.gitErrorCode = "RemoteConnectionError" /* RemoteConnectionError */;
            }
            else if (/^fatal: The current branch .* has no upstream branch/.test(err.stderr || '')) {
                err.gitErrorCode = "NoUpstreamBranch" /* NoUpstreamBranch */;
            }
            throw err;
        }
    }
    async blame(path) {
        try {
            const args = ['blame'];
            args.push(path);
            let result = await this.run(args);
            return result.stdout.trim();
        }
        catch (err) {
            if (/^fatal: no such path/.test(err.stderr || '')) {
                err.gitErrorCode = "NoPathFound" /* NoPathFound */;
            }
            throw err;
        }
    }
    async createStash(message, includeUntracked) {
        try {
            const args = ['stash', 'push'];
            if (includeUntracked) {
                args.push('-u');
            }
            if (message) {
                args.push('-m', message);
            }
            await this.run(args);
        }
        catch (err) {
            if (/No local changes to save/.test(err.stderr || '')) {
                err.gitErrorCode = "NoLocalChanges" /* NoLocalChanges */;
            }
            throw err;
        }
    }
    async popStash(index) {
        const args = ['stash', 'pop'];
        await this.popOrApplyStash(args, index);
    }
    async applyStash(index) {
        const args = ['stash', 'apply'];
        await this.popOrApplyStash(args, index);
    }
    async popOrApplyStash(args, index) {
        try {
            if (typeof index === 'number') {
                args.push(`stash@{${index}}`);
            }
            await this.run(args);
        }
        catch (err) {
            if (/No stash found/.test(err.stderr || '')) {
                err.gitErrorCode = "NoStashFound" /* NoStashFound */;
            }
            else if (/error: Your local changes to the following files would be overwritten/.test(err.stderr || '')) {
                err.gitErrorCode = "LocalChangesOverwritten" /* LocalChangesOverwritten */;
            }
            else if (/^CONFLICT/m.test(err.stdout || '')) {
                err.gitErrorCode = "StashConflict" /* StashConflict */;
            }
            throw err;
        }
    }
    getStatus(limit = 5000) {
        return new Promise((c, e) => {
            const parser = new GitStatusParser();
            const env = { GIT_OPTIONAL_LOCKS: '0' };
            const child = this.stream(['status', '-z', '-u'], { env });
            const onExit = (exitCode) => {
                if (exitCode !== 0) {
                    const stderr = stderrData.join('');
                    return e(new GitError({
                        message: 'Failed to execute git',
                        stderr,
                        exitCode,
                        gitErrorCode: getGitErrorCode(stderr),
                        gitCommand: 'status'
                    }));
                }
                c({ status: parser.status, didHitLimit: false });
            };
            const onStdoutData = (raw) => {
                parser.update(raw);
                if (parser.status.length > limit) {
                    child.removeListener('exit', onExit);
                    child.stdout.removeListener('data', onStdoutData);
                    child.kill();
                    c({ status: parser.status.slice(0, limit), didHitLimit: true });
                }
            };
            child.stdout.setEncoding('utf8');
            child.stdout.on('data', onStdoutData);
            const stderrData = [];
            child.stderr.setEncoding('utf8');
            child.stderr.on('data', raw => stderrData.push(raw));
            child.on('error', cpErrorHandler(e));
            child.on('exit', onExit);
        });
    }
    async getHEAD() {
        try {
            const result = await this.run(['symbolic-ref', '--short', 'HEAD']);
            if (!result.stdout) {
                throw new Error('Not in a branch');
            }
            return { name: result.stdout.trim(), commit: undefined, type: 0 /* Head */ };
        }
        catch (err) {
            const result = await this.run(['rev-parse', 'HEAD']);
            if (!result.stdout) {
                throw new Error('Error parsing HEAD');
            }
            return { name: undefined, commit: result.stdout.trim(), type: 0 /* Head */ };
        }
    }
    async getRefs() {
        const result = await this.run(['for-each-ref', '--format', '%(refname) %(objectname)', '--sort', '-committerdate']);
        const fn = (line) => {
            let match;
            if (match = /^refs\/heads\/([^ ]+) ([0-9a-f]{40})$/.exec(line)) {
                return { name: match[1], commit: match[2], type: 0 /* Head */ };
            }
            else if (match = /^refs\/remotes\/([^/]+)\/([^ ]+) ([0-9a-f]{40})$/.exec(line)) {
                return { name: `${match[1]}/${match[2]}`, commit: match[3], type: 1 /* RemoteHead */, remote: match[1] };
            }
            else if (match = /^refs\/tags\/([^ ]+) ([0-9a-f]{40})$/.exec(line)) {
                return { name: match[1], commit: match[2], type: 2 /* Tag */ };
            }
            return null;
        };
        return result.stdout.trim().split('\n')
            .filter(line => !!line)
            .map(fn)
            .filter(ref => !!ref);
    }
    async getStashes() {
        const result = await this.run(['stash', 'list']);
        const regex = /^stash@{(\d+)}:(.+)$/;
        const rawStashes = result.stdout.trim().split('\n')
            .filter(b => !!b)
            .map(line => regex.exec(line))
            .filter(g => !!g)
            .map(([, index, description]) => ({ index: parseInt(index), description }));
        return rawStashes;
    }
    async getRemotes() {
        const result = await this.run(['remote', '--verbose']);
        const lines = result.stdout.trim().split('\n').filter(l => !!l);
        const remotes = [];
        for (const line of lines) {
            const parts = line.split(/\s/);
            const [name, url, type] = parts;
            let remote = remotes.find(r => r.name === name);
            if (!remote) {
                remote = { name, isReadOnly: false };
                remotes.push(remote);
            }
            if (/fetch/i.test(type)) {
                remote.fetchUrl = url;
            }
            else if (/push/i.test(type)) {
                remote.pushUrl = url;
            }
            else {
                remote.fetchUrl = url;
                remote.pushUrl = url;
            }
            // https://github.com/Microsoft/vscode/issues/45271
            remote.isReadOnly = remote.pushUrl === undefined || remote.pushUrl === 'no_push';
        }
        return remotes;
    }
    async getBranch(name) {
        if (name === 'HEAD') {
            return this.getHEAD();
        }
        else if (/^@/.test(name)) {
            const symbolicFullNameResult = await this.run(['rev-parse', '--symbolic-full-name', name]);
            const symbolicFullName = symbolicFullNameResult.stdout.trim();
            name = symbolicFullName || name;
        }
        const result = await this.run(['rev-parse', name]);
        if (!result.stdout) {
            return Promise.reject(new Error('No such branch'));
        }
        const commit = result.stdout.trim();
        try {
            const res2 = await this.run(['rev-parse', '--symbolic-full-name', name + '@{u}']);
            const fullUpstream = res2.stdout.trim();
            const match = /^refs\/remotes\/([^/]+)\/(.+)$/.exec(fullUpstream);
            if (!match) {
                throw new Error(`Could not parse upstream branch: ${fullUpstream}`);
            }
            const upstream = { remote: match[1], name: match[2] };
            const res3 = await this.run(['rev-list', '--left-right', name + '...' + fullUpstream]);
            let ahead = 0, behind = 0;
            let i = 0;
            while (i < res3.stdout.length) {
                switch (res3.stdout.charAt(i)) {
                    case '<':
                        ahead++;
                        break;
                    case '>':
                        behind++;
                        break;
                    default:
                        i++;
                        break;
                }
                while (res3.stdout.charAt(i++) !== '\n') { /* no-op */ }
            }
            return { name, type: 0 /* Head */, commit, upstream, ahead, behind };
        }
        catch (err) {
            return { name, type: 0 /* Head */, commit };
        }
    }
    async getCommitTemplate() {
        try {
            const result = await this.run(['config', '--get', 'commit.template']);
            if (!result.stdout) {
                return '';
            }
            // https://github.com/git/git/blob/3a0f269e7c82aa3a87323cb7ae04ac5f129f036b/path.c#L612
            const homedir = os.homedir();
            let templatePath = result.stdout.trim()
                .replace(/^~([^\/]*)\//, (_, user) => `${user ? path.join(path.dirname(homedir), user) : homedir}/`);
            if (!path.isAbsolute(templatePath)) {
                templatePath = path.join(this.repositoryRoot, templatePath);
            }
            const raw = await readfile(templatePath, 'utf8');
            return raw.replace(/^\s*#.*$\n?/gm, '').trim();
        }
        catch (err) {
            return '';
        }
    }
    async getCommit(ref) {
        const result = await this.run(['show', '-s', `--format=${COMMIT_FORMAT}`, ref]);
        return parseGitCommit(result.stdout) || Promise.reject('bad commit format');
    }
    async updateSubmodules(paths) {
        const args = ['submodule', 'update', '--', ...paths];
        await this.run(args);
    }
    async getSubmodules() {
        const gitmodulesPath = path.join(this.root, '.gitmodules');
        try {
            const gitmodulesRaw = await readfile(gitmodulesPath, 'utf8');
            return parseGitmodules(gitmodulesRaw);
        }
        catch (err) {
            if (/ENOENT/.test(err.message)) {
                return [];
            }
            throw err;
        }
    }
}
exports.Repository = Repository;
//# sourceMappingURL=git.js.map