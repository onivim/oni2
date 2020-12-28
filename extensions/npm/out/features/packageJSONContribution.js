"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.PackageJSONContribution = void 0;
const vscode_1 = require("vscode");
const cp = require("child_process");
const nls = require("vscode-nls");
const path_1 = require("path");
const localize = nls.loadMessageBundle();
const LIMIT = 40;
const SCOPED_LIMIT = 250;
const USER_AGENT = 'Visual Studio Code';
class PackageJSONContribution {
    constructor(xhr, canRunNPM) {
        this.xhr = xhr;
        this.canRunNPM = canRunNPM;
        this.mostDependedOn = ['lodash', 'async', 'underscore', 'request', 'commander', 'express', 'debug', 'chalk', 'colors', 'q', 'coffee-script',
            'mkdirp', 'optimist', 'through2', 'yeoman-generator', 'moment', 'bluebird', 'glob', 'gulp-util', 'minimist', 'cheerio', 'pug', 'redis', 'node-uuid',
            'socket', 'io', 'uglify-js', 'winston', 'through', 'fs-extra', 'handlebars', 'body-parser', 'rimraf', 'mime', 'semver', 'mongodb', 'jquery',
            'grunt', 'connect', 'yosay', 'underscore', 'string', 'xml2js', 'ejs', 'mongoose', 'marked', 'extend', 'mocha', 'superagent', 'js-yaml', 'xtend',
            'shelljs', 'gulp', 'yargs', 'browserify', 'minimatch', 'react', 'less', 'prompt', 'inquirer', 'ws', 'event-stream', 'inherits', 'mysql', 'esprima',
            'jsdom', 'stylus', 'when', 'readable-stream', 'aws-sdk', 'concat-stream', 'chai', 'Thenable', 'wrench'];
        this.knownScopes = ['@types', '@angular', '@babel', '@nuxtjs', '@vue', '@bazel'];
    }
    getDocumentSelector() {
        return [{ language: 'json', scheme: '*', pattern: '**/package.json' }];
    }
    collectDefaultSuggestions(_resource, result) {
        const defaultValue = {
            'name': '${1:name}',
            'description': '${2:description}',
            'authors': '${3:author}',
            'version': '${4:1.0.0}',
            'main': '${5:pathToMain}',
            'dependencies': {}
        };
        const proposal = new vscode_1.CompletionItem(localize('json.package.default', 'Default package.json'));
        proposal.kind = vscode_1.CompletionItemKind.Module;
        proposal.insertText = new vscode_1.SnippetString(JSON.stringify(defaultValue, null, '\t'));
        result.add(proposal);
        return Promise.resolve(null);
    }
    isEnabled() {
        return this.canRunNPM || this.onlineEnabled();
    }
    onlineEnabled() {
        return !!vscode_1.workspace.getConfiguration('npm').get('fetchOnlinePackageInfo');
    }
    collectPropertySuggestions(_resource, location, currentWord, addValue, isLast, collector) {
        if (!this.isEnabled()) {
            return null;
        }
        if ((location.matches(['dependencies']) || location.matches(['devDependencies']) || location.matches(['optionalDependencies']) || location.matches(['peerDependencies']))) {
            let queryUrl;
            if (currentWord.length > 0) {
                if (currentWord[0] === '@') {
                    if (currentWord.indexOf('/') !== -1) {
                        return this.collectScopedPackages(currentWord, addValue, isLast, collector);
                    }
                    for (let scope of this.knownScopes) {
                        const proposal = new vscode_1.CompletionItem(scope);
                        proposal.kind = vscode_1.CompletionItemKind.Property;
                        proposal.insertText = new vscode_1.SnippetString().appendText(`"${scope}/`).appendTabstop().appendText('"');
                        proposal.filterText = JSON.stringify(scope);
                        proposal.documentation = '';
                        proposal.command = {
                            title: '',
                            command: 'editor.action.triggerSuggest'
                        };
                        collector.add(proposal);
                    }
                    collector.setAsIncomplete();
                }
                queryUrl = `https://api.npms.io/v2/search/suggestions?size=${LIMIT}&q=${encodeURIComponent(currentWord)}`;
                return this.xhr({
                    url: queryUrl,
                    agent: USER_AGENT
                }).then((success) => {
                    if (success.status === 200) {
                        try {
                            const obj = JSON.parse(success.responseText);
                            if (obj && Array.isArray(obj)) {
                                const results = obj;
                                for (const result of results) {
                                    this.processPackage(result.package, addValue, isLast, collector);
                                }
                                if (results.length === LIMIT) {
                                    collector.setAsIncomplete();
                                }
                            }
                        }
                        catch (e) {
                            // ignore
                        }
                    }
                    else {
                        collector.error(localize('json.npm.error.repoaccess', 'Request to the NPM repository failed: {0}', success.responseText));
                        return 0;
                    }
                    return undefined;
                }, (error) => {
                    collector.error(localize('json.npm.error.repoaccess', 'Request to the NPM repository failed: {0}', error.responseText));
                    return 0;
                });
            }
            else {
                this.mostDependedOn.forEach((name) => {
                    const insertText = new vscode_1.SnippetString().appendText(JSON.stringify(name));
                    if (addValue) {
                        insertText.appendText(': "').appendTabstop().appendText('"');
                        if (!isLast) {
                            insertText.appendText(',');
                        }
                    }
                    const proposal = new vscode_1.CompletionItem(name);
                    proposal.kind = vscode_1.CompletionItemKind.Property;
                    proposal.insertText = insertText;
                    proposal.filterText = JSON.stringify(name);
                    proposal.documentation = '';
                    collector.add(proposal);
                });
                this.collectScopedPackages(currentWord, addValue, isLast, collector);
                collector.setAsIncomplete();
                return Promise.resolve(null);
            }
        }
        return null;
    }
    collectScopedPackages(currentWord, addValue, isLast, collector) {
        let segments = currentWord.split('/');
        if (segments.length === 2 && segments[0].length > 1) {
            let scope = segments[0].substr(1);
            let name = segments[1];
            if (name.length < 4) {
                name = '';
            }
            let queryUrl = `https://api.npms.io/v2/search?q=scope:${scope}%20${name}&size=250`;
            return this.xhr({
                url: queryUrl,
                agent: USER_AGENT
            }).then((success) => {
                if (success.status === 200) {
                    try {
                        const obj = JSON.parse(success.responseText);
                        if (obj && Array.isArray(obj.results)) {
                            const objects = obj.results;
                            for (let object of objects) {
                                this.processPackage(object.package, addValue, isLast, collector);
                            }
                            if (objects.length === SCOPED_LIMIT) {
                                collector.setAsIncomplete();
                            }
                        }
                    }
                    catch (e) {
                        // ignore
                    }
                }
                else {
                    collector.error(localize('json.npm.error.repoaccess', 'Request to the NPM repository failed: {0}', success.responseText));
                }
                return null;
            });
        }
        return Promise.resolve(null);
    }
    async collectValueSuggestions(resource, location, result) {
        if (!this.isEnabled()) {
            return null;
        }
        if ((location.matches(['dependencies', '*']) || location.matches(['devDependencies', '*']) || location.matches(['optionalDependencies', '*']) || location.matches(['peerDependencies', '*']))) {
            const currentKey = location.path[location.path.length - 1];
            if (typeof currentKey === 'string') {
                const info = await this.fetchPackageInfo(currentKey, resource);
                if (info && info.version) {
                    let name = JSON.stringify(info.version);
                    let proposal = new vscode_1.CompletionItem(name);
                    proposal.kind = vscode_1.CompletionItemKind.Property;
                    proposal.insertText = name;
                    proposal.documentation = localize('json.npm.latestversion', 'The currently latest version of the package');
                    result.add(proposal);
                    name = JSON.stringify('^' + info.version);
                    proposal = new vscode_1.CompletionItem(name);
                    proposal.kind = vscode_1.CompletionItemKind.Property;
                    proposal.insertText = name;
                    proposal.documentation = localize('json.npm.majorversion', 'Matches the most recent major version (1.x.x)');
                    result.add(proposal);
                    name = JSON.stringify('~' + info.version);
                    proposal = new vscode_1.CompletionItem(name);
                    proposal.kind = vscode_1.CompletionItemKind.Property;
                    proposal.insertText = name;
                    proposal.documentation = localize('json.npm.minorversion', 'Matches the most recent minor version (1.2.x)');
                    result.add(proposal);
                }
            }
        }
        return null;
    }
    getDocumentation(description, version, homepage) {
        const str = new vscode_1.MarkdownString();
        if (description) {
            str.appendText(description);
        }
        if (version) {
            str.appendText('\n\n');
            str.appendText(localize('json.npm.version.hover', 'Latest version: {0}', version));
        }
        if (homepage) {
            str.appendText('\n\n');
            str.appendText(homepage);
        }
        return str;
    }
    resolveSuggestion(resource, item) {
        if (item.kind === vscode_1.CompletionItemKind.Property && !item.documentation) {
            return this.fetchPackageInfo(item.label, resource).then(info => {
                if (info) {
                    item.documentation = this.getDocumentation(info.description, info.version, info.homepage);
                    return item;
                }
                return null;
            });
        }
        return null;
    }
    isValidNPMName(name) {
        // following rules from https://github.com/npm/validate-npm-package-name
        if (!name || name.length > 214 || name.match(/^[_.]/)) {
            return false;
        }
        const match = name.match(/^(?:@([^/]+?)[/])?([^/]+?)$/);
        if (match) {
            const scope = match[1];
            if (scope && encodeURIComponent(scope) !== scope) {
                return false;
            }
            const name = match[2];
            return encodeURIComponent(name) === name;
        }
        return false;
    }
    async fetchPackageInfo(pack, resource) {
        if (!this.isValidNPMName(pack)) {
            return undefined; // avoid unnecessary lookups
        }
        let info;
        if (this.canRunNPM) {
            info = await this.npmView(pack, resource);
        }
        if (!info && this.onlineEnabled()) {
            info = await this.npmjsView(pack);
        }
        return info;
    }
    npmView(pack, resource) {
        return new Promise((resolve, _reject) => {
            const args = ['view', '--json', pack, 'description', 'dist-tags.latest', 'homepage', 'version'];
            let cwd = resource && resource.scheme === 'file' ? path_1.dirname(resource.fsPath) : undefined;
            cp.execFile(process.platform === 'win32' ? 'npm.cmd' : 'npm', args, { cwd }, (error, stdout) => {
                if (!error) {
                    try {
                        const content = JSON.parse(stdout);
                        resolve({
                            description: content['description'],
                            version: content['dist-tags.latest'] || content['version'],
                            homepage: content['homepage']
                        });
                        return;
                    }
                    catch (e) {
                        // ignore
                    }
                }
                resolve(undefined);
            });
        });
    }
    async npmjsView(pack) {
        var _a, _b;
        const queryUrl = 'https://api.npms.io/v2/package/' + encodeURIComponent(pack);
        try {
            const success = await this.xhr({
                url: queryUrl,
                agent: USER_AGENT
            });
            const obj = JSON.parse(success.responseText);
            const metadata = (_a = obj === null || obj === void 0 ? void 0 : obj.collected) === null || _a === void 0 ? void 0 : _a.metadata;
            if (metadata) {
                return {
                    description: metadata.description || '',
                    version: metadata.version,
                    homepage: ((_b = metadata.links) === null || _b === void 0 ? void 0 : _b.homepage) || ''
                };
            }
        }
        catch (e) {
            //ignore
        }
        return undefined;
    }
    getInfoContribution(resource, location) {
        if (!this.isEnabled()) {
            return null;
        }
        if ((location.matches(['dependencies', '*']) || location.matches(['devDependencies', '*']) || location.matches(['optionalDependencies', '*']) || location.matches(['peerDependencies', '*']))) {
            const pack = location.path[location.path.length - 1];
            if (typeof pack === 'string') {
                return this.fetchPackageInfo(pack, resource).then(info => {
                    if (info) {
                        return [this.getDocumentation(info.description, info.version, info.homepage)];
                    }
                    return null;
                });
            }
        }
        return null;
    }
    processPackage(pack, addValue, isLast, collector) {
        var _a;
        if (pack && pack.name) {
            const name = pack.name;
            const insertText = new vscode_1.SnippetString().appendText(JSON.stringify(name));
            if (addValue) {
                insertText.appendText(': "');
                if (pack.version) {
                    insertText.appendVariable('version', pack.version);
                }
                else {
                    insertText.appendTabstop();
                }
                insertText.appendText('"');
                if (!isLast) {
                    insertText.appendText(',');
                }
            }
            const proposal = new vscode_1.CompletionItem(name);
            proposal.kind = vscode_1.CompletionItemKind.Property;
            proposal.insertText = insertText;
            proposal.filterText = JSON.stringify(name);
            proposal.documentation = this.getDocumentation(pack.description, pack.version, (_a = pack === null || pack === void 0 ? void 0 : pack.links) === null || _a === void 0 ? void 0 : _a.homepage);
            collector.add(proposal);
        }
    }
}
exports.PackageJSONContribution = PackageJSONContribution;
//# sourceMappingURL=packageJSONContribution.js.map