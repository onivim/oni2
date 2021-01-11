"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const mergeConflictParser_1 = require("./mergeConflictParser");
const delayer_1 = require("./delayer");
class ScanTask {
    constructor(delayTime, initialOrigin) {
        this.origins = new Set();
        this.origins.add(initialOrigin);
        this.delayTask = new delayer_1.Delayer(delayTime);
    }
    addOrigin(name) {
        if (this.origins.has(name)) {
            return false;
        }
        return false;
    }
    hasOrigin(name) {
        return this.origins.has(name);
    }
}
class OriginDocumentMergeConflictTracker {
    constructor(parent, origin) {
        this.parent = parent;
        this.origin = origin;
    }
    getConflicts(document) {
        return this.parent.getConflicts(document, this.origin);
    }
    isPending(document) {
        return this.parent.isPending(document, this.origin);
    }
    forget(document) {
        this.parent.forget(document);
    }
}
class DocumentMergeConflictTracker {
    constructor() {
        this.cache = new Map();
        this.delayExpireTime = 0;
    }
    getConflicts(document, origin) {
        // Attempt from cache
        let key = this.getCacheKey(document);
        if (!key) {
            // Document doesn't have a uri, can't cache it, so return
            return Promise.resolve(this.getConflictsOrEmpty(document, [origin]));
        }
        let cacheItem = this.cache.get(key);
        if (!cacheItem) {
            cacheItem = new ScanTask(this.delayExpireTime, origin);
            this.cache.set(key, cacheItem);
        }
        else {
            cacheItem.addOrigin(origin);
        }
        return cacheItem.delayTask.trigger(() => {
            let conflicts = this.getConflictsOrEmpty(document, Array.from(cacheItem.origins));
            if (this.cache) {
                this.cache.delete(key);
            }
            return conflicts;
        });
    }
    isPending(document, origin) {
        if (!document) {
            return false;
        }
        let key = this.getCacheKey(document);
        if (!key) {
            return false;
        }
        const task = this.cache.get(key);
        if (!task) {
            return false;
        }
        return task.hasOrigin(origin);
    }
    createTracker(origin) {
        return new OriginDocumentMergeConflictTracker(this, origin);
    }
    forget(document) {
        let key = this.getCacheKey(document);
        if (key) {
            this.cache.delete(key);
        }
    }
    dispose() {
        this.cache.clear();
    }
    getConflictsOrEmpty(document, _origins) {
        const containsConflict = mergeConflictParser_1.MergeConflictParser.containsConflict(document);
        if (!containsConflict) {
            return [];
        }
        const conflicts = mergeConflictParser_1.MergeConflictParser.scanDocument(document);
        return conflicts;
    }
    getCacheKey(document) {
        if (document.uri) {
            return document.uri.toString();
        }
        return null;
    }
}
exports.default = DocumentMergeConflictTracker;
//# sourceMappingURL=documentTracker.js.map