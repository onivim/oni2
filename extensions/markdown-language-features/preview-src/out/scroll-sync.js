"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.getLineElementForFragment = exports.getEditorLineNumberForPageOffset = exports.scrollToRevealSourceLine = exports.getLineElementsAtPageOffset = exports.getElementsForSourceLine = void 0;
const settings_1 = require("./settings");
const codeLineClass = 'code-line';
function clamp(min, max, value) {
    return Math.min(max, Math.max(min, value));
}
function clampLine(line) {
    return clamp(0, settings_1.getSettings().lineCount - 1, line);
}
const getCodeLineElements = (() => {
    let elements;
    return () => {
        if (!elements) {
            elements = [{ element: document.body, line: 0 }];
            for (const element of document.getElementsByClassName(codeLineClass)) {
                const line = +element.getAttribute('data-line');
                if (isNaN(line)) {
                    continue;
                }
                if (element.tagName === 'CODE' && element.parentElement && element.parentElement.tagName === 'PRE') {
                    // Fenched code blocks are a special case since the `code-line` can only be marked on
                    // the `<code>` element and not the parent `<pre>` element.
                    elements.push({ element: element.parentElement, line });
                }
                else {
                    elements.push({ element: element, line });
                }
            }
        }
        return elements;
    };
})();
/**
 * Find the html elements that map to a specific target line in the editor.
 *
 * If an exact match, returns a single element. If the line is between elements,
 * returns the element prior to and the element after the given line.
 */
function getElementsForSourceLine(targetLine) {
    const lineNumber = Math.floor(targetLine);
    const lines = getCodeLineElements();
    let previous = lines[0] || null;
    for (const entry of lines) {
        if (entry.line === lineNumber) {
            return { previous: entry, next: undefined };
        }
        else if (entry.line > lineNumber) {
            return { previous, next: entry };
        }
        previous = entry;
    }
    return { previous };
}
exports.getElementsForSourceLine = getElementsForSourceLine;
/**
 * Find the html elements that are at a specific pixel offset on the page.
 */
function getLineElementsAtPageOffset(offset) {
    const lines = getCodeLineElements();
    const position = offset - window.scrollY;
    let lo = -1;
    let hi = lines.length - 1;
    while (lo + 1 < hi) {
        const mid = Math.floor((lo + hi) / 2);
        const bounds = getElementBounds(lines[mid]);
        if (bounds.top + bounds.height >= position) {
            hi = mid;
        }
        else {
            lo = mid;
        }
    }
    const hiElement = lines[hi];
    const hiBounds = getElementBounds(hiElement);
    if (hi >= 1 && hiBounds.top > position) {
        const loElement = lines[lo];
        return { previous: loElement, next: hiElement };
    }
    if (hi > 1 && hi < lines.length && hiBounds.top + hiBounds.height > position) {
        return { previous: hiElement, next: lines[hi + 1] };
    }
    return { previous: hiElement };
}
exports.getLineElementsAtPageOffset = getLineElementsAtPageOffset;
function getElementBounds({ element }) {
    const myBounds = element.getBoundingClientRect();
    // Some code line elements may contain other code line elements.
    // In those cases, only take the height up to that child.
    const codeLineChild = element.querySelector(`.${codeLineClass}`);
    if (codeLineChild) {
        const childBounds = codeLineChild.getBoundingClientRect();
        const height = Math.max(1, (childBounds.top - myBounds.top));
        return {
            top: myBounds.top,
            height: height
        };
    }
    return myBounds;
}
/**
 * Attempt to reveal the element for a source line in the editor.
 */
function scrollToRevealSourceLine(line) {
    if (!settings_1.getSettings().scrollPreviewWithEditor) {
        return;
    }
    if (line <= 0) {
        window.scroll(window.scrollX, 0);
        return;
    }
    const { previous, next } = getElementsForSourceLine(line);
    if (!previous) {
        return;
    }
    let scrollTo = 0;
    const rect = getElementBounds(previous);
    const previousTop = rect.top;
    if (next && next.line !== previous.line) {
        // Between two elements. Go to percentage offset between them.
        const betweenProgress = (line - previous.line) / (next.line - previous.line);
        const elementOffset = next.element.getBoundingClientRect().top - previousTop;
        scrollTo = previousTop + betweenProgress * elementOffset;
    }
    else {
        const progressInElement = line - Math.floor(line);
        scrollTo = previousTop + (rect.height * progressInElement);
    }
    window.scroll(window.scrollX, Math.max(1, window.scrollY + scrollTo));
}
exports.scrollToRevealSourceLine = scrollToRevealSourceLine;
function getEditorLineNumberForPageOffset(offset) {
    const { previous, next } = getLineElementsAtPageOffset(offset);
    if (previous) {
        const previousBounds = getElementBounds(previous);
        const offsetFromPrevious = (offset - window.scrollY - previousBounds.top);
        if (next) {
            const progressBetweenElements = offsetFromPrevious / (getElementBounds(next).top - previousBounds.top);
            const line = previous.line + progressBetweenElements * (next.line - previous.line);
            return clampLine(line);
        }
        else {
            const progressWithinElement = offsetFromPrevious / (previousBounds.height);
            const line = previous.line + progressWithinElement;
            return clampLine(line);
        }
    }
    return null;
}
exports.getEditorLineNumberForPageOffset = getEditorLineNumberForPageOffset;
/**
 * Try to find the html element by using a fragment id
 */
function getLineElementForFragment(fragment) {
    return getCodeLineElements().find((element) => {
        return element.element.id === fragment;
    });
}
exports.getLineElementForFragment = getLineElementForFragment;
//# sourceMappingURL=scroll-sync.js.map