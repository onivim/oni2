/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { ExtensionContext, Uri } from 'vscode';
import { LanguageClientOptions } from 'vscode-languageclient';
import { startClient, LanguageClientConstructor } from '../cssClient';
import { LanguageClient } from 'vscode-languageclient/browser';

declare const Worker: {
	new(stringUrl: string): any;
};
declare const TextDecoder: {
	new(encoding?: string): { decode(buffer: ArrayBuffer): string; };
};

// this method is called when vs code is activated
export function activate(context: ExtensionContext) {
	const serverMain = Uri.joinPath(context.extensionUri, 'server/dist/browser/cssServerMain.js');
	try {
		const worker = new Worker(serverMain.toString());
		const newLanguageClient: LanguageClientConstructor = (id: string, name: string, clientOptions: LanguageClientOptions) => {
			return new LanguageClient(id, name, clientOptions, worker);
		};

		startClient(context, newLanguageClient, { TextDecoder });

	} catch (e) {
		console.log(e);
	}
}
