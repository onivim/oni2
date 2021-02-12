"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const activeLineMarker_1 = require("./activeLineMarker");
const events_1 = require("./events");
const messaging_1 = require("./messaging");
const scroll_sync_1 = require("./scroll-sync");
const settings_1 = require("./settings");
const throttle = require("lodash.throttle");
let scrollDisabled = true;
const marker = new activeLineMarker_1.ActiveLineMarker();
const settings = settings_1.getSettings();
const vscode = acquireVsCodeApi();
const originalState = vscode.getState();
const state = {
    ...(typeof originalState === 'object' ? originalState : {}),
    ...settings_1.getData('data-state')
};
// Make sure to sync VS Code state here
vscode.setState(state);
const messaging = messaging_1.createPosterForVsCode(vscode);
window.cspAlerter.setPoster(messaging);
window.styleLoadingMonitor.setPoster(messaging);
window.onload = () => {
    updateImageSizes();
};
events_1.onceDocumentLoaded(() => {
    const scrollProgress = state.scrollProgress;
    if (typeof scrollProgress === 'number' && !settings.fragment) {
        setImmediate(() => {
            scrollDisabled = true;
            window.scrollTo(0, scrollProgress * document.body.clientHeight);
        });
        return;
    }
    if (settings.scrollPreviewWithEditor) {
        setImmediate(() => {
            // Try to scroll to fragment if available
            if (settings.fragment) {
                state.fragment = undefined;
                vscode.setState(state);
                const element = scroll_sync_1.getLineElementForFragment(settings.fragment);
                if (element) {
                    scrollDisabled = true;
                    scroll_sync_1.scrollToRevealSourceLine(element.line);
                }
            }
            else {
                if (!isNaN(settings.line)) {
                    scrollDisabled = true;
                    scroll_sync_1.scrollToRevealSourceLine(settings.line);
                }
            }
        });
    }
});
const onUpdateView = (() => {
    const doScroll = throttle((line) => {
        scrollDisabled = true;
        scroll_sync_1.scrollToRevealSourceLine(line);
    }, 50);
    return (line) => {
        if (!isNaN(line)) {
            state.line = line;
            doScroll(line);
        }
    };
})();
let updateImageSizes = throttle(() => {
    const imageInfo = [];
    let images = document.getElementsByTagName('img');
    if (images) {
        let i;
        for (i = 0; i < images.length; i++) {
            const img = images[i];
            if (img.classList.contains('loading')) {
                img.classList.remove('loading');
            }
            imageInfo.push({
                id: img.id,
                height: img.height,
                width: img.width
            });
        }
        messaging.postMessage('cacheImageSizes', imageInfo);
    }
}, 50);
window.addEventListener('resize', () => {
    scrollDisabled = true;
    updateScrollProgress();
    updateImageSizes();
}, true);
window.addEventListener('message', event => {
    if (event.data.source !== settings.source) {
        return;
    }
    switch (event.data.type) {
        case 'onDidChangeTextEditorSelection':
            marker.onDidChangeTextEditorSelection(event.data.line);
            break;
        case 'updateView':
            onUpdateView(event.data.line);
            break;
    }
}, false);
document.addEventListener('dblclick', event => {
    if (!settings.doubleClickToSwitchToEditor) {
        return;
    }
    // Ignore clicks on links
    for (let node = event.target; node; node = node.parentNode) {
        if (node.tagName === 'A') {
            return;
        }
    }
    const offset = event.pageY;
    const line = scroll_sync_1.getEditorLineNumberForPageOffset(offset);
    if (typeof line === 'number' && !isNaN(line)) {
        messaging.postMessage('didClick', { line: Math.floor(line) });
    }
});
const passThroughLinkSchemes = ['http:', 'https:', 'mailto:', 'vscode:', 'vscode-insiders:'];
document.addEventListener('click', event => {
    if (!event) {
        return;
    }
    let node = event.target;
    while (node) {
        if (node.tagName && node.tagName === 'A' && node.href) {
            if (node.getAttribute('href').startsWith('#')) {
                return;
            }
            let hrefText = node.getAttribute('data-href');
            if (!hrefText) {
                // Pass through known schemes
                if (passThroughLinkSchemes.some(scheme => node.href.startsWith(scheme))) {
                    return;
                }
                hrefText = node.getAttribute('href');
            }
            // If original link doesn't look like a url, delegate back to VS Code to resolve
            if (!/^[a-z\-]+:/i.test(hrefText)) {
                messaging.postMessage('openLink', { href: hrefText });
                event.preventDefault();
                event.stopPropagation();
                return;
            }
            return;
        }
        node = node.parentNode;
    }
}, true);
window.addEventListener('scroll', throttle(() => {
    updateScrollProgress();
    if (scrollDisabled) {
        scrollDisabled = false;
    }
    else {
        const line = scroll_sync_1.getEditorLineNumberForPageOffset(window.scrollY);
        if (typeof line === 'number' && !isNaN(line)) {
            messaging.postMessage('revealLine', { line });
        }
    }
}, 50));
function updateScrollProgress() {
    state.scrollProgress = window.scrollY / document.body.clientHeight;
    vscode.setState(state);
}
//# sourceMappingURL=index.js.map