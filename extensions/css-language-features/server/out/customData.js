"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
const fs = require("fs");
function getDataProviders(dataPaths) {
    const providers = dataPaths.map(p => {
        if (fs.existsSync(p)) {
            const data = parseCSSData(fs.readFileSync(p, 'utf-8'));
            return {
                provideProperties: () => data.properties || [],
                provideAtDirectives: () => data.atDirectives || [],
                providePseudoClasses: () => data.pseudoClasses || [],
                providePseudoElements: () => data.pseudoElements || []
            };
        }
        else {
            return {
                provideProperties: () => [],
                provideAtDirectives: () => [],
                providePseudoClasses: () => [],
                providePseudoElements: () => []
            };
        }
    });
    return providers;
}
exports.getDataProviders = getDataProviders;
function parseCSSData(source) {
    let rawData;
    try {
        rawData = JSON.parse(source);
    }
    catch (err) {
        return {
            version: 1
        };
    }
    return {
        version: 1,
        properties: rawData.properties || [],
        atDirectives: rawData.atDirectives || [],
        pseudoClasses: rawData.pseudoClasses || [],
        pseudoElements: rawData.pseudoElements || []
    };
}
//# sourceMappingURL=customData.js.map