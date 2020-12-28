"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.register = void 0;
const vscode = require("vscode");
const api_1 = require("../utils/api");
const arrays_1 = require("../utils/arrays");
const dependentRegistration_1 = require("../utils/dependentRegistration");
const typeConverters = require("../utils/typeConverters");
let TypeScriptFoldingProvider = /** @class */ (() => {
    class TypeScriptFoldingProvider {
        constructor(client) {
            this.client = client;
        }
        async provideFoldingRanges(document, _context, token) {
            const file = this.client.toOpenedFilePath(document);
            if (!file) {
                return;
            }
            const args = { file };
            const response = await this.client.execute('getOutliningSpans', args, token);
            if (response.type !== 'response' || !response.body) {
                return;
            }
            return arrays_1.coalesce(response.body.map(span => this.convertOutliningSpan(span, document)));
        }
        convertOutliningSpan(span, document) {
            const range = typeConverters.Range.fromTextSpan(span.textSpan);
            const kind = TypeScriptFoldingProvider.getFoldingRangeKind(span);
            // Workaround for #49904
            if (span.kind === 'comment') {
                const line = document.lineAt(range.start.line).text;
                if (line.match(/\/\/\s*#endregion/gi)) {
                    return undefined;
                }
            }
            const start = range.start.line;
            // workaround for #47240
            const end = (range.end.character > 0 && ['}', ']'].includes(document.getText(new vscode.Range(range.end.translate(0, -1), range.end))))
                ? Math.max(range.end.line - 1, range.start.line)
                : range.end.line;
            return new vscode.FoldingRange(start, end, kind);
        }
        static getFoldingRangeKind(span) {
            switch (span.kind) {
                case 'comment': return vscode.FoldingRangeKind.Comment;
                case 'region': return vscode.FoldingRangeKind.Region;
                case 'imports': return vscode.FoldingRangeKind.Imports;
                case 'code':
                default: return undefined;
            }
        }
    }
    TypeScriptFoldingProvider.minVersion = api_1.default.v280;
    return TypeScriptFoldingProvider;
})();
function register(selector, client) {
    return new dependentRegistration_1.VersionDependentRegistration(client, TypeScriptFoldingProvider.minVersion, () => {
        return vscode.languages.registerFoldingRangeProvider(selector, new TypeScriptFoldingProvider(client));
    });
}
exports.register = register;
//# sourceMappingURL=folding.js.map