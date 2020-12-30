"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.MarkdownContentProvider = void 0;
const path = require("path");
const vscode = require("vscode");
const nls = require("vscode-nls");
const localize = nls.loadMessageBundle();
/**
 * Strings used inside the markdown preview.
 *
 * Stored here and then injected in the preview so that they
 * can be localized using our normal localization process.
 */
const previewStrings = {
    cspAlertMessageText: localize('preview.securityMessage.text', 'Some content has been disabled in this document'),
    cspAlertMessageTitle: localize('preview.securityMessage.title', 'Potentially unsafe or insecure content has been disabled in the markdown preview. Change the Markdown preview security setting to allow insecure content or enable scripts'),
    cspAlertMessageLabel: localize('preview.securityMessage.label', 'Content Disabled Security Warning')
};
function escapeAttribute(value) {
    return value.toString().replace(/"/g, '&quot;');
}
class MarkdownContentProvider {
    constructor(engine, context, cspArbiter, contributionProvider, logger) {
        this.engine = engine;
        this.context = context;
        this.cspArbiter = cspArbiter;
        this.contributionProvider = contributionProvider;
        this.logger = logger;
    }
    async provideTextDocumentContent(markdownDocument, resourceProvider, previewConfigurations, initialLine = undefined, state) {
        const sourceUri = markdownDocument.uri;
        const config = previewConfigurations.loadAndCacheConfiguration(sourceUri);
        const initialData = {
            source: sourceUri.toString(),
            line: initialLine,
            lineCount: markdownDocument.lineCount,
            scrollPreviewWithEditor: config.scrollPreviewWithEditor,
            scrollEditorWithPreview: config.scrollEditorWithPreview,
            doubleClickToSwitchToEditor: config.doubleClickToSwitchToEditor,
            disableSecurityWarnings: this.cspArbiter.shouldDisableSecurityWarnings(),
            webviewResourceRoot: resourceProvider.asWebviewUri(markdownDocument.uri).toString(),
        };
        this.logger.log('provideTextDocumentContent', initialData);
        // Content Security Policy
        const nonce = new Date().getTime() + '' + new Date().getMilliseconds();
        const csp = this.getCsp(resourceProvider, sourceUri, nonce);
        const body = await this.engine.render(markdownDocument);
        return `<!DOCTYPE html>
			<html style="${escapeAttribute(this.getSettingsOverrideStyles(config))}">
			<head>
				<meta http-equiv="Content-type" content="text/html;charset=UTF-8">
				${csp}
				<meta id="vscode-markdown-preview-data"
					data-settings="${escapeAttribute(JSON.stringify(initialData))}"
					data-strings="${escapeAttribute(JSON.stringify(previewStrings))}"
					data-state="${escapeAttribute(JSON.stringify(state || {}))}">
				<script src="${this.extensionResourcePath(resourceProvider, 'pre.js')}" nonce="${nonce}"></script>
				${this.getStyles(resourceProvider, sourceUri, config, state)}
				<base href="${resourceProvider.asWebviewUri(markdownDocument.uri)}">
			</head>
			<body class="vscode-body ${config.scrollBeyondLastLine ? 'scrollBeyondLastLine' : ''} ${config.wordWrap ? 'wordWrap' : ''} ${config.markEditorSelection ? 'showEditorSelection' : ''}">
				${body}
				<div class="code-line" data-line="${markdownDocument.lineCount}"></div>
				${this.getScripts(resourceProvider, nonce)}
			</body>
			</html>`;
    }
    provideFileNotFoundContent(resource) {
        const resourcePath = path.basename(resource.fsPath);
        const body = localize('preview.notFound', '{0} cannot be found', resourcePath);
        return `<!DOCTYPE html>
			<html>
			<body class="vscode-body">
				${body}
			</body>
			</html>`;
    }
    extensionResourcePath(resourceProvider, mediaFile) {
        const webviewResource = resourceProvider.asWebviewUri(vscode.Uri.joinPath(this.context.extensionUri, 'media', mediaFile));
        return webviewResource.toString();
    }
    fixHref(resourceProvider, resource, href) {
        if (!href) {
            return href;
        }
        if (href.startsWith('http:') || href.startsWith('https:') || href.startsWith('file:')) {
            return href;
        }
        // Assume it must be a local file
        if (path.isAbsolute(href)) {
            return resourceProvider.asWebviewUri(vscode.Uri.file(href)).toString();
        }
        // Use a workspace relative path if there is a workspace
        const root = vscode.workspace.getWorkspaceFolder(resource);
        if (root) {
            return resourceProvider.asWebviewUri(vscode.Uri.joinPath(root.uri, href)).toString();
        }
        // Otherwise look relative to the markdown file
        return resourceProvider.asWebviewUri(vscode.Uri.file(path.join(path.dirname(resource.fsPath), href))).toString();
    }
    computeCustomStyleSheetIncludes(resourceProvider, resource, config) {
        if (!Array.isArray(config.styles)) {
            return '';
        }
        const out = [];
        for (const style of config.styles) {
            out.push(`<link rel="stylesheet" class="code-user-style" data-source="${escapeAttribute(style)}" href="${escapeAttribute(this.fixHref(resourceProvider, resource, style))}" type="text/css" media="screen">`);
        }
        return out.join('\n');
    }
    getSettingsOverrideStyles(config) {
        return [
            config.fontFamily ? `--markdown-font-family: ${config.fontFamily};` : '',
            isNaN(config.fontSize) ? '' : `--markdown-font-size: ${config.fontSize}px;`,
            isNaN(config.lineHeight) ? '' : `--markdown-line-height: ${config.lineHeight};`,
        ].join(' ');
    }
    getImageStabilizerStyles(state) {
        let ret = '<style>\n';
        if (state && state.imageInfo) {
            state.imageInfo.forEach((imgInfo) => {
                ret += `#${imgInfo.id}.loading {
					height: ${imgInfo.height}px;
					width: ${imgInfo.width}px;
				}\n`;
            });
        }
        ret += '</style>\n';
        return ret;
    }
    getStyles(resourceProvider, resource, config, state) {
        const baseStyles = [];
        for (const resource of this.contributionProvider.contributions.previewStyles) {
            baseStyles.push(`<link rel="stylesheet" type="text/css" href="${escapeAttribute(resourceProvider.asWebviewUri(resource))}">`);
        }
        return `${baseStyles.join('\n')}
			${this.computeCustomStyleSheetIncludes(resourceProvider, resource, config)}
			${this.getImageStabilizerStyles(state)}`;
    }
    getScripts(resourceProvider, nonce) {
        const out = [];
        for (const resource of this.contributionProvider.contributions.previewScripts) {
            out.push(`<script async
				src="${escapeAttribute(resourceProvider.asWebviewUri(resource))}"
				nonce="${nonce}"
				charset="UTF-8"></script>`);
        }
        return out.join('\n');
    }
    getCsp(provider, resource, nonce) {
        const rule = provider.cspSource;
        switch (this.cspArbiter.getSecurityLevelForResource(resource)) {
            case 1 /* AllowInsecureContent */:
                return `<meta http-equiv="Content-Security-Policy" content="default-src 'none'; img-src 'self' ${rule} http: https: data:; media-src 'self' ${rule} http: https: data:; script-src 'nonce-${nonce}'; style-src 'self' ${rule} 'unsafe-inline' http: https: data:; font-src 'self' ${rule} http: https: data:;">`;
            case 3 /* AllowInsecureLocalContent */:
                return `<meta http-equiv="Content-Security-Policy" content="default-src 'none'; img-src 'self' ${rule} https: data: http://localhost:* http://127.0.0.1:*; media-src 'self' ${rule} https: data: http://localhost:* http://127.0.0.1:*; script-src 'nonce-${nonce}'; style-src 'self' ${rule} 'unsafe-inline' https: data: http://localhost:* http://127.0.0.1:*; font-src 'self' ${rule} https: data: http://localhost:* http://127.0.0.1:*;">`;
            case 2 /* AllowScriptsAndAllContent */:
                return '<meta http-equiv="Content-Security-Policy" content="">';
            case 0 /* Strict */:
            default:
                return `<meta http-equiv="Content-Security-Policy" content="default-src 'none'; img-src 'self' ${rule} https: data:; media-src 'self' ${rule} https: data:; script-src 'nonce-${nonce}'; style-src 'self' ${rule} 'unsafe-inline' https: data:; font-src 'self' ${rule} https: data:;">`;
        }
    }
}
exports.MarkdownContentProvider = MarkdownContentProvider;
//# sourceMappingURL=previewContentProvider.js.map