"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.RenderDocument = void 0;
class RenderDocument {
    constructor(engine) {
        this.engine = engine;
        this.id = 'markdown.api.render';
    }
    async execute(document) {
        return this.engine.render(document);
    }
}
exports.RenderDocument = RenderDocument;
//# sourceMappingURL=renderDocument.js.map