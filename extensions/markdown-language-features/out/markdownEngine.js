"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.MarkdownEngine = void 0;
const path = require("path");
const vscode = require("vscode");
const hash_1 = require("./util/hash");
const links_1 = require("./util/links");
const UNICODE_NEWLINE_REGEX = /\u2028|\u2029/g;
class TokenCache {
    tryGetCached(document, config) {
        if (this.cachedDocument
            && this.cachedDocument.uri.toString() === document.uri.toString()
            && this.cachedDocument.version === document.version
            && this.cachedDocument.config.breaks === config.breaks
            && this.cachedDocument.config.linkify === config.linkify) {
            return this.tokens;
        }
        return undefined;
    }
    update(document, config, tokens) {
        this.cachedDocument = {
            uri: document.uri,
            version: document.version,
            config,
        };
        this.tokens = tokens;
    }
    clean() {
        this.cachedDocument = undefined;
        this.tokens = undefined;
    }
}
class MarkdownEngine {
    constructor(contributionProvider, slugifier) {
        this.contributionProvider = contributionProvider;
        this.slugifier = slugifier;
        this._slugCount = new Map();
        this._tokenCache = new TokenCache();
        contributionProvider.onContributionsChanged(() => {
            // Markdown plugin contributions may have changed
            this.md = undefined;
        });
    }
    async getEngine(config) {
        if (!this.md) {
            this.md = Promise.resolve().then(() => require('markdown-it')).then(async (markdownIt) => {
                let md = markdownIt(await getMarkdownOptions(() => md));
                for (const plugin of this.contributionProvider.contributions.markdownItPlugins.values()) {
                    try {
                        md = (await plugin)(md);
                    }
                    catch (_a) {
                        // noop
                    }
                }
                const frontMatterPlugin = require('markdown-it-front-matter');
                // Extract rules from front matter plugin and apply at a lower precedence
                let fontMatterRule;
                frontMatterPlugin({
                    block: {
                        ruler: {
                            before: (_id, _id2, rule) => { fontMatterRule = rule; }
                        }
                    }
                }, () => { });
                md.block.ruler.before('fence', 'front_matter', fontMatterRule, {
                    alt: ['paragraph', 'reference', 'blockquote', 'list']
                });
                for (const renderName of ['paragraph_open', 'heading_open', 'image', 'code_block', 'fence', 'blockquote_open', 'list_item_open']) {
                    this.addLineNumberRenderer(md, renderName);
                }
                this.addImageStabilizer(md);
                this.addFencedRenderer(md);
                this.addLinkNormalizer(md);
                this.addLinkValidator(md);
                this.addNamedHeaders(md);
                this.addLinkRenderer(md);
                return md;
            });
        }
        const md = await this.md;
        md.set(config);
        return md;
    }
    tokenizeDocument(document, config, engine) {
        const cached = this._tokenCache.tryGetCached(document, config);
        if (cached) {
            return cached;
        }
        this.currentDocument = document.uri;
        const tokens = this.tokenizeString(document.getText(), engine);
        this._tokenCache.update(document, config, tokens);
        return tokens;
    }
    tokenizeString(text, engine) {
        this._slugCount = new Map();
        return engine.parse(text.replace(UNICODE_NEWLINE_REGEX, ''), {});
    }
    async render(input) {
        const config = this.getConfig(typeof input === 'string' ? undefined : input.uri);
        const engine = await this.getEngine(config);
        const tokens = typeof input === 'string'
            ? this.tokenizeString(input, engine)
            : this.tokenizeDocument(input, config, engine);
        return engine.renderer.render(tokens, {
            ...engine.options,
            ...config
        }, {});
    }
    async parse(document) {
        const config = this.getConfig(document.uri);
        const engine = await this.getEngine(config);
        return this.tokenizeDocument(document, config, engine);
    }
    cleanCache() {
        this._tokenCache.clean();
    }
    getConfig(resource) {
        const config = vscode.workspace.getConfiguration('markdown', resource);
        return {
            breaks: config.get('preview.breaks', false),
            linkify: config.get('preview.linkify', true)
        };
    }
    addLineNumberRenderer(md, ruleName) {
        const original = md.renderer.rules[ruleName];
        md.renderer.rules[ruleName] = (tokens, idx, options, env, self) => {
            const token = tokens[idx];
            if (token.map && token.map.length) {
                token.attrSet('data-line', token.map[0]);
                token.attrJoin('class', 'code-line');
            }
            if (original) {
                return original(tokens, idx, options, env, self);
            }
            else {
                return self.renderToken(tokens, idx, options, env, self);
            }
        };
    }
    addImageStabilizer(md) {
        const original = md.renderer.rules.image;
        md.renderer.rules.image = (tokens, idx, options, env, self) => {
            const token = tokens[idx];
            token.attrJoin('class', 'loading');
            const src = token.attrGet('src');
            if (src) {
                const imgHash = hash_1.hash(src);
                token.attrSet('id', `image-hash-${imgHash}`);
            }
            if (original) {
                return original(tokens, idx, options, env, self);
            }
            else {
                return self.renderToken(tokens, idx, options, env, self);
            }
        };
    }
    addFencedRenderer(md) {
        const original = md.renderer.rules['fenced'];
        md.renderer.rules['fenced'] = (tokens, idx, options, env, self) => {
            const token = tokens[idx];
            if (token.map && token.map.length) {
                token.attrJoin('class', 'hljs');
            }
            return original(tokens, idx, options, env, self);
        };
    }
    addLinkNormalizer(md) {
        const normalizeLink = md.normalizeLink;
        md.normalizeLink = (link) => {
            try {
                // Normalize VS Code schemes to target the current version
                if (links_1.isOfScheme(links_1.Schemes.vscode, link) || links_1.isOfScheme(links_1.Schemes['vscode-insiders'], link)) {
                    return normalizeLink(vscode.Uri.parse(link).with({ scheme: vscode.env.uriScheme }).toString());
                }
                // If original link doesn't look like a url with a scheme, assume it must be a link to a file in workspace
                if (!/^[a-z\-]+:/i.test(link)) {
                    // Use a fake scheme for parsing
                    let uri = vscode.Uri.parse('markdown-link:' + link);
                    // Relative paths should be resolved correctly inside the preview but we need to
                    // handle absolute paths specially (for images) to resolve them relative to the workspace root
                    if (uri.path[0] === '/') {
                        const root = vscode.workspace.getWorkspaceFolder(this.currentDocument);
                        if (root) {
                            const fileUri = vscode.Uri.joinPath(root.uri, uri.fsPath);
                            uri = fileUri.with({
                                scheme: uri.scheme,
                                fragment: uri.fragment,
                                query: uri.query,
                            });
                        }
                    }
                    const extname = path.extname(uri.fsPath);
                    if (uri.fragment && (extname === '' || links_1.MarkdownFileExtensions.includes(extname))) {
                        uri = uri.with({
                            fragment: this.slugifier.fromHeading(uri.fragment).value
                        });
                    }
                    return normalizeLink(uri.toString(true).replace(/^markdown-link:/, ''));
                }
            }
            catch (e) {
                // noop
            }
            return normalizeLink(link);
        };
    }
    addLinkValidator(md) {
        const validateLink = md.validateLink;
        md.validateLink = (link) => {
            // support file:// links
            return validateLink(link)
                || links_1.isOfScheme(links_1.Schemes.file, link)
                || links_1.isOfScheme(links_1.Schemes.vscode, link)
                || links_1.isOfScheme(links_1.Schemes['vscode-insiders'], link)
                || /^data:image\/.*?;/.test(link);
        };
    }
    addNamedHeaders(md) {
        const original = md.renderer.rules.heading_open;
        md.renderer.rules.heading_open = (tokens, idx, options, env, self) => {
            const title = tokens[idx + 1].children.reduce((acc, t) => acc + t.content, '');
            let slug = this.slugifier.fromHeading(title);
            if (this._slugCount.has(slug.value)) {
                const count = this._slugCount.get(slug.value);
                this._slugCount.set(slug.value, count + 1);
                slug = this.slugifier.fromHeading(slug.value + '-' + (count + 1));
            }
            else {
                this._slugCount.set(slug.value, 0);
            }
            tokens[idx].attrs = tokens[idx].attrs || [];
            tokens[idx].attrs.push(['id', slug.value]);
            if (original) {
                return original(tokens, idx, options, env, self);
            }
            else {
                return self.renderToken(tokens, idx, options, env, self);
            }
        };
    }
    addLinkRenderer(md) {
        const old_render = md.renderer.rules.link_open || ((tokens, idx, options, _env, self) => {
            return self.renderToken(tokens, idx, options);
        });
        md.renderer.rules.link_open = (tokens, idx, options, env, self) => {
            const token = tokens[idx];
            const hrefIndex = token.attrIndex('href');
            if (hrefIndex >= 0) {
                const href = token.attrs[hrefIndex][1];
                token.attrPush(['data-href', href]);
            }
            return old_render(tokens, idx, options, env, self);
        };
    }
}
exports.MarkdownEngine = MarkdownEngine;
async function getMarkdownOptions(md) {
    const hljs = await Promise.resolve().then(() => require('highlight.js'));
    return {
        html: true,
        highlight: (str, lang) => {
            lang = normalizeHighlightLang(lang);
            if (lang && hljs.getLanguage(lang)) {
                try {
                    return `<div>${hljs.highlight(lang, str, true).value}</div>`;
                }
                catch (error) { }
            }
            return `<code><div>${md().utils.escapeHtml(str)}</div></code>`;
        }
    };
}
function normalizeHighlightLang(lang) {
    switch (lang && lang.toLowerCase()) {
        case 'tsx':
        case 'typescriptreact':
            // Workaround for highlight not supporting tsx: https://github.com/isagalaev/highlight.js/issues/1155
            return 'jsx';
        case 'json5':
        case 'jsonc':
            return 'json';
        case 'c#':
        case 'csharp':
            return 'cs';
        default:
            return lang;
    }
}
//# sourceMappingURL=markdownEngine.js.map