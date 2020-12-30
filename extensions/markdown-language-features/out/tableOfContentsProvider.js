"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.TableOfContentsProvider = void 0;
const vscode = require("vscode");
const slugify_1 = require("./slugify");
class TableOfContentsProvider {
    constructor(engine, document) {
        this.engine = engine;
        this.document = document;
    }
    async getToc() {
        if (!this.toc) {
            try {
                this.toc = await this.buildToc(this.document);
            }
            catch (e) {
                this.toc = [];
            }
        }
        return this.toc;
    }
    async lookup(fragment) {
        const toc = await this.getToc();
        const slug = slugify_1.githubSlugifier.fromHeading(fragment);
        return toc.find(entry => entry.slug.equals(slug));
    }
    async buildToc(document) {
        const toc = [];
        const tokens = await this.engine.parse(document);
        const existingSlugEntries = new Map();
        for (const heading of tokens.filter(token => token.type === 'heading_open')) {
            const lineNumber = heading.map[0];
            const line = document.lineAt(lineNumber);
            let slug = slugify_1.githubSlugifier.fromHeading(line.text);
            const existingSlugEntry = existingSlugEntries.get(slug.value);
            if (existingSlugEntry) {
                ++existingSlugEntry.count;
                slug = slugify_1.githubSlugifier.fromHeading(slug.value + '-' + existingSlugEntry.count);
            }
            else {
                existingSlugEntries.set(slug.value, { count: 0 });
            }
            toc.push({
                slug,
                text: TableOfContentsProvider.getHeaderText(line.text),
                level: TableOfContentsProvider.getHeaderLevel(heading.markup),
                line: lineNumber,
                location: new vscode.Location(document.uri, new vscode.Range(lineNumber, 0, lineNumber, line.text.length))
            });
        }
        // Get full range of section
        return toc.map((entry, startIndex) => {
            let end = undefined;
            for (let i = startIndex + 1; i < toc.length; ++i) {
                if (toc[i].level <= entry.level) {
                    end = toc[i].line - 1;
                    break;
                }
            }
            const endLine = end !== null && end !== void 0 ? end : document.lineCount - 1;
            return {
                ...entry,
                location: new vscode.Location(document.uri, new vscode.Range(entry.location.range.start, new vscode.Position(endLine, document.lineAt(endLine).text.length)))
            };
        });
    }
    static getHeaderLevel(markup) {
        if (markup === '=') {
            return 1;
        }
        else if (markup === '-') {
            return 2;
        }
        else { // '#', '##', ...
            return markup.length;
        }
    }
    static getHeaderText(header) {
        return header.replace(/^\s*#+\s*(.*?)\s*#*$/, (_, word) => word.trim());
    }
}
exports.TableOfContentsProvider = TableOfContentsProvider;
//# sourceMappingURL=tableOfContentsProvider.js.map