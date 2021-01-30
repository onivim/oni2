"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.ShowLockedPreviewToSideCommand = exports.ShowPreviewToSideCommand = exports.ShowPreviewCommand = void 0;
const vscode = require("vscode");
async function showPreview(webviewManager, telemetryReporter, uri, previewSettings) {
    let resource = uri;
    if (!(resource instanceof vscode.Uri)) {
        if (vscode.window.activeTextEditor) {
            // we are relaxed and don't check for markdown files
            resource = vscode.window.activeTextEditor.document.uri;
        }
    }
    if (!(resource instanceof vscode.Uri)) {
        if (!vscode.window.activeTextEditor) {
            // this is most likely toggling the preview
            return vscode.commands.executeCommand('markdown.showSource');
        }
        // nothing found that could be shown or toggled
        return;
    }
    const resourceColumn = (vscode.window.activeTextEditor && vscode.window.activeTextEditor.viewColumn) || vscode.ViewColumn.One;
    webviewManager.openDynamicPreview(resource, {
        resourceColumn: resourceColumn,
        previewColumn: previewSettings.sideBySide ? resourceColumn + 1 : resourceColumn,
        locked: !!previewSettings.locked
    });
    telemetryReporter.sendTelemetryEvent('openPreview', {
        where: previewSettings.sideBySide ? 'sideBySide' : 'inPlace',
        how: (uri instanceof vscode.Uri) ? 'action' : 'pallete'
    });
}
class ShowPreviewCommand {
    constructor(webviewManager, telemetryReporter) {
        this.webviewManager = webviewManager;
        this.telemetryReporter = telemetryReporter;
        this.id = 'markdown.showPreview';
    }
    execute(mainUri, allUris, previewSettings) {
        for (const uri of Array.isArray(allUris) ? allUris : [mainUri]) {
            showPreview(this.webviewManager, this.telemetryReporter, uri, {
                sideBySide: false,
                locked: previewSettings && previewSettings.locked
            });
        }
    }
}
exports.ShowPreviewCommand = ShowPreviewCommand;
class ShowPreviewToSideCommand {
    constructor(webviewManager, telemetryReporter) {
        this.webviewManager = webviewManager;
        this.telemetryReporter = telemetryReporter;
        this.id = 'markdown.showPreviewToSide';
    }
    execute(uri, previewSettings) {
        showPreview(this.webviewManager, this.telemetryReporter, uri, {
            sideBySide: true,
            locked: previewSettings && previewSettings.locked
        });
    }
}
exports.ShowPreviewToSideCommand = ShowPreviewToSideCommand;
class ShowLockedPreviewToSideCommand {
    constructor(webviewManager, telemetryReporter) {
        this.webviewManager = webviewManager;
        this.telemetryReporter = telemetryReporter;
        this.id = 'markdown.showLockedPreviewToSide';
    }
    execute(uri) {
        showPreview(this.webviewManager, this.telemetryReporter, uri, {
            sideBySide: true,
            locked: true
        });
    }
}
exports.ShowLockedPreviewToSideCommand = ShowLockedPreviewToSideCommand;
//# sourceMappingURL=showPreview.js.map