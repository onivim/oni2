"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.StyleLoadingMonitor = void 0;
class StyleLoadingMonitor {
    constructor() {
        this.unloadedStyles = [];
        this.finishedLoading = false;
        const onStyleLoadError = (event) => {
            const source = event.target.dataset.source;
            this.unloadedStyles.push(source);
        };
        window.addEventListener('DOMContentLoaded', () => {
            for (const link of document.getElementsByClassName('code-user-style')) {
                if (link.dataset.source) {
                    link.onerror = onStyleLoadError;
                }
            }
        });
        window.addEventListener('load', () => {
            if (!this.unloadedStyles.length) {
                return;
            }
            this.finishedLoading = true;
            if (this.poster) {
                this.poster.postMessage('previewStyleLoadError', { unloadedStyles: this.unloadedStyles });
            }
        });
    }
    setPoster(poster) {
        this.poster = poster;
        if (this.finishedLoading) {
            poster.postMessage('previewStyleLoadError', { unloadedStyles: this.unloadedStyles });
        }
    }
}
exports.StyleLoadingMonitor = StyleLoadingMonitor;
//# sourceMappingURL=loading.js.map