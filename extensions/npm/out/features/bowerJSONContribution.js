"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.BowerJSONContribution = void 0;
const vscode_1 = require("vscode");
const nls = require("vscode-nls");
const localize = nls.loadMessageBundle();
const USER_AGENT = 'Visual Studio Code';
class BowerJSONContribution {
    constructor(xhr) {
        this.topRanked = ['twitter', 'bootstrap', 'angular-1.1.6', 'angular-latest', 'angulerjs', 'd3', 'myjquery', 'jq', 'abcdef1234567890', 'jQuery', 'jquery-1.11.1', 'jquery',
            'sushi-vanilla-x-data', 'font-awsome', 'Font-Awesome', 'font-awesome', 'fontawesome', 'html5-boilerplate', 'impress.js', 'homebrew',
            'backbone', 'moment1', 'momentjs', 'moment', 'linux', 'animate.css', 'animate-css', 'reveal.js', 'jquery-file-upload', 'blueimp-file-upload', 'threejs', 'express', 'chosen',
            'normalize-css', 'normalize.css', 'semantic', 'semantic-ui', 'Semantic-UI', 'modernizr', 'underscore', 'underscore1',
            'material-design-icons', 'ionic', 'chartjs', 'Chart.js', 'nnnick-chartjs', 'select2-ng', 'select2-dist', 'phantom', 'skrollr', 'scrollr', 'less.js', 'leancss', 'parser-lib',
            'hui', 'bootstrap-languages', 'async', 'gulp', 'jquery-pjax', 'coffeescript', 'hammer.js', 'ace', 'leaflet', 'jquery-mobile', 'sweetalert', 'typeahead.js', 'soup', 'typehead.js',
            'sails', 'codeigniter2'];
        this.xhr = xhr;
    }
    getDocumentSelector() {
        return [{ language: 'json', scheme: '*', pattern: '**/bower.json' }, { language: 'json', scheme: '*', pattern: '**/.bower.json' }];
    }
    isEnabled() {
        return !!vscode_1.workspace.getConfiguration('npm').get('fetchOnlinePackageInfo');
    }
    collectDefaultSuggestions(_resource, collector) {
        const defaultValue = {
            'name': '${1:name}',
            'description': '${2:description}',
            'authors': ['${3:author}'],
            'version': '${4:1.0.0}',
            'main': '${5:pathToMain}',
            'dependencies': {}
        };
        const proposal = new vscode_1.CompletionItem(localize('json.bower.default', 'Default bower.json'));
        proposal.kind = vscode_1.CompletionItemKind.Class;
        proposal.insertText = new vscode_1.SnippetString(JSON.stringify(defaultValue, null, '\t'));
        collector.add(proposal);
        return Promise.resolve(null);
    }
    collectPropertySuggestions(_resource, location, currentWord, addValue, isLast, collector) {
        if (!this.isEnabled()) {
            return null;
        }
        if ((location.matches(['dependencies']) || location.matches(['devDependencies']))) {
            if (currentWord.length > 0) {
                const queryUrl = 'https://registry.bower.io/packages/search/' + encodeURIComponent(currentWord);
                return this.xhr({
                    url: queryUrl,
                    agent: USER_AGENT
                }).then((success) => {
                    if (success.status === 200) {
                        try {
                            const obj = JSON.parse(success.responseText);
                            if (Array.isArray(obj)) {
                                const results = obj;
                                for (const result of results) {
                                    const name = result.name;
                                    const description = result.description || '';
                                    const insertText = new vscode_1.SnippetString().appendText(JSON.stringify(name));
                                    if (addValue) {
                                        insertText.appendText(': ').appendPlaceholder('latest');
                                        if (!isLast) {
                                            insertText.appendText(',');
                                        }
                                    }
                                    const proposal = new vscode_1.CompletionItem(name);
                                    proposal.kind = vscode_1.CompletionItemKind.Property;
                                    proposal.insertText = insertText;
                                    proposal.filterText = JSON.stringify(name);
                                    proposal.documentation = description;
                                    collector.add(proposal);
                                }
                                collector.setAsIncomplete();
                            }
                        }
                        catch (e) {
                            // ignore
                        }
                    }
                    else {
                        collector.error(localize('json.bower.error.repoaccess', 'Request to the bower repository failed: {0}', success.responseText));
                        return 0;
                    }
                    return undefined;
                }, (error) => {
                    collector.error(localize('json.bower.error.repoaccess', 'Request to the bower repository failed: {0}', error.responseText));
                    return 0;
                });
            }
            else {
                this.topRanked.forEach((name) => {
                    const insertText = new vscode_1.SnippetString().appendText(JSON.stringify(name));
                    if (addValue) {
                        insertText.appendText(': ').appendPlaceholder('latest');
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
                collector.setAsIncomplete();
                return Promise.resolve(null);
            }
        }
        return null;
    }
    collectValueSuggestions(_resource, location, collector) {
        if (!this.isEnabled()) {
            return null;
        }
        if ((location.matches(['dependencies', '*']) || location.matches(['devDependencies', '*']))) {
            // not implemented. Could be do done calling the bower command. Waiting for web API: https://github.com/bower/registry/issues/26
            const proposal = new vscode_1.CompletionItem(localize('json.bower.latest.version', 'latest'));
            proposal.insertText = new vscode_1.SnippetString('"${1:latest}"');
            proposal.filterText = '""';
            proposal.kind = vscode_1.CompletionItemKind.Value;
            proposal.documentation = 'The latest version of the package';
            collector.add(proposal);
        }
        return null;
    }
    resolveSuggestion(_resource, item) {
        if (item.kind === vscode_1.CompletionItemKind.Property && item.documentation === '') {
            return this.getInfo(item.label).then(documentation => {
                if (documentation) {
                    item.documentation = documentation;
                    return item;
                }
                return null;
            });
        }
        return null;
    }
    getInfo(pack) {
        const queryUrl = 'https://registry.bower.io/packages/' + encodeURIComponent(pack);
        return this.xhr({
            url: queryUrl,
            agent: USER_AGENT
        }).then((success) => {
            try {
                const obj = JSON.parse(success.responseText);
                if (obj && obj.url) {
                    let url = obj.url;
                    if (url.indexOf('git://') === 0) {
                        url = url.substring(6);
                    }
                    if (url.length >= 4 && url.substr(url.length - 4) === '.git') {
                        url = url.substring(0, url.length - 4);
                    }
                    return url;
                }
            }
            catch (e) {
                // ignore
            }
            return undefined;
        }, () => {
            return undefined;
        });
    }
    getInfoContribution(_resource, location) {
        if (!this.isEnabled()) {
            return null;
        }
        if ((location.matches(['dependencies', '*']) || location.matches(['devDependencies', '*']))) {
            const pack = location.path[location.path.length - 1];
            if (typeof pack === 'string') {
                return this.getInfo(pack).then(documentation => {
                    if (documentation) {
                        const str = new vscode_1.MarkdownString();
                        str.appendText(documentation);
                        return [str];
                    }
                    return null;
                });
            }
        }
        return null;
    }
}
exports.BowerJSONContribution = BowerJSONContribution;
//# sourceMappingURL=bowerJSONContribution.js.map