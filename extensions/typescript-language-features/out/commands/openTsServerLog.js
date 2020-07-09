"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
exports.OpenTsServerLogCommand = void 0;
class OpenTsServerLogCommand {
    constructor(lazyClientHost) {
        this.lazyClientHost = lazyClientHost;
        this.id = 'typescript.openTsServerLog';
    }
    execute() {
        this.lazyClientHost.value.serviceClient.openTsServerLogFile();
    }
}
exports.OpenTsServerLogCommand = OpenTsServerLogCommand;
//# sourceMappingURL=openTsServerLog.js.map