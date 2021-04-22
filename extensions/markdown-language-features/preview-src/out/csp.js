"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.CspAlerter = void 0;
const settings_1 = require("./settings");
const strings_1 = require("./strings");
/**
 * Shows an alert when there is a content security policy violation.
 */
class CspAlerter {
    constructor() {
        this.didShow = false;
        this.didHaveCspWarning = false;
        document.addEventListener('securitypolicyviolation', () => {
            this.onCspWarning();
        });
        window.addEventListener('message', (event) => {
            if (event && event.data && event.data.name === 'vscode-did-block-svg') {
                this.onCspWarning();
            }
        });
    }
    setPoster(poster) {
        this.messaging = poster;
        if (this.didHaveCspWarning) {
            this.showCspWarning();
        }
    }
    onCspWarning() {
        this.didHaveCspWarning = true;
        this.showCspWarning();
    }
    showCspWarning() {
        const strings = strings_1.getStrings();
        const settings = settings_1.getSettings();
        if (this.didShow || settings.disableSecurityWarnings || !this.messaging) {
            return;
        }
        this.didShow = true;
        const notification = document.createElement('a');
        notification.innerText = strings.cspAlertMessageText;
        notification.setAttribute('id', 'code-csp-warning');
        notification.setAttribute('title', strings.cspAlertMessageTitle);
        notification.setAttribute('role', 'button');
        notification.setAttribute('aria-label', strings.cspAlertMessageLabel);
        notification.onclick = () => {
            this.messaging.postMessage('showPreviewSecuritySelector', { source: settings.source });
        };
        document.body.appendChild(notification);
    }
}
exports.CspAlerter = CspAlerter;
//# sourceMappingURL=csp.js.map