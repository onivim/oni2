"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.githubSlugifier = exports.Slug = void 0;
class Slug {
    constructor(value) {
        this.value = value;
    }
    equals(other) {
        return this.value === other.value;
    }
}
exports.Slug = Slug;
exports.githubSlugifier = new class {
    fromHeading(heading) {
        const slugifiedHeading = encodeURI(heading.trim()
            .toLowerCase()
            .replace(/\s+/g, '-') // Replace whitespace with -
            .replace(/[\]\[\!\'\#\$\%\&\(\)\*\+\,\.\/\:\;\<\=\>\?\@\\\^\_\{\|\}\~\`。，、；：？！…—·ˉ¨‘’“”々～‖∶＂＇｀｜〃〔〕〈〉《》「」『』．〖〗【】（）［］｛｝]/g, '') // Remove known punctuators
            .replace(/^\-+/, '') // Remove leading -
            .replace(/\-+$/, '') // Remove trailing -
        );
        return new Slug(slugifiedHeading);
    }
};
//# sourceMappingURL=slugify.js.map