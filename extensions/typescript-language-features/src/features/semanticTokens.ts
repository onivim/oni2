/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import * as vscode from 'vscode';
import { ITypeScriptServiceClient, ExecConfig, ServerResponse } from '../typescriptService';
import * as Proto from '../protocol';
import { VersionDependentRegistration } from '../utils/dependentRegistration';
import API from '../utils/api';

// all constants are const
import { TokenType, TokenModifier, TokenEncodingConsts, VersionRequirement } from 'typescript-vscode-sh-plugin/lib/constants';

const minTypeScriptVersion = API.fromVersionString(`${VersionRequirement.major}.${VersionRequirement.minor}`);

export function register(selector: vscode.DocumentSelector, client: ITypeScriptServiceClient) {
	return new VersionDependentRegistration(client, minTypeScriptVersion, () => {
		const provider = new DocumentSemanticTokensProvider(client);
		return vscode.Disposable.from(
			vscode.languages.registerDocumentSemanticTokensProvider(selector, provider, provider.getLegend()),
			vscode.languages.registerDocumentRangeSemanticTokensProvider(selector, provider, provider.getLegend()),
		);
	});
}


// as we don't do deltas, for performance reasons, don't compute semantic tokens for documents above that limit
const CONTENT_LENGTH_LIMIT = 100000;

/**
 * Prototype of a DocumentSemanticTokensProvider, relying on the experimental `encodedSemanticClassifications-full` request from the TypeScript server.
 * As the results retured by the TypeScript server are limited, we also add a Typescript plugin (typescript-vscode-sh-plugin) to enrich the returned token.
 * See https://github.com/aeschli/typescript-vscode-sh-plugin.
 */
class DocumentSemanticTokensProvider implements vscode.DocumentSemanticTokensProvider, vscode.DocumentRangeSemanticTokensProvider {

	constructor(private readonly client: ITypeScriptServiceClient) {
	}

	getLegend(): vscode.SemanticTokensLegend {
		return new vscode.SemanticTokensLegend(tokenTypes, tokenModifiers);
	}

	async provideDocumentSemanticTokens(document: vscode.TextDocument, token: vscode.CancellationToken): Promise<vscode.SemanticTokens | null> {
		const file = this.client.toOpenedFilePath(document);
		if (!file || document.getText().length > CONTENT_LENGTH_LIMIT) {
			return null;
		}
		return this._provideSemanticTokens(document, { file, start: 0, length: document.getText().length }, token);
	}

	async provideDocumentRangeSemanticTokens(document: vscode.TextDocument, range: vscode.Range, token: vscode.CancellationToken): Promise<vscode.SemanticTokens | null> {
		const file = this.client.toOpenedFilePath(document);
		if (!file || document.getText().length > CONTENT_LENGTH_LIMIT) {
			return null;
		}
		const start = document.offsetAt(range.start);
		const length = document.offsetAt(range.end) - start;
		return this._provideSemanticTokens(document, { file, start, length }, token);
	}

	async _provideSemanticTokens(document: vscode.TextDocument, requestArg: ExperimentalProtocol.EncodedSemanticClassificationsRequestArgs, token: vscode.CancellationToken): Promise<vscode.SemanticTokens | null> {
		const file = this.client.toOpenedFilePath(document);
		if (!file) {
			return null;
		}

		let versionBeforeRequest = document.version;

		const response = await (this.client as ExperimentalProtocol.IExtendedTypeScriptServiceClient).execute('encodedSemanticClassifications-full', requestArg, token);
		if (response.type !== 'response' || !response.body) {
			return null;
		}

		const versionAfterRequest = document.version;

		if (versionBeforeRequest !== versionAfterRequest) {
			// cannot convert result's offsets to (line;col) values correctly
			// a new request will come in soon...
			//
			// here we cannot return null, because returning null would remove all semantic tokens.
			// we must throw to indicate that the semantic tokens should not be removed.
			// using the string busy here because it is not logged to error telemetry if the error text contains busy.

			// as the new request will come in right after our response, we first wait for the document activity to stop
			await waitForDocumentChangesToEnd(document);

			throw new Error('busy');
		}

		const tokenSpan = response.body.spans;

		const builder = new vscode.SemanticTokensBuilder();
		let i = 0;
		while (i < tokenSpan.length) {
			const offset = tokenSpan[i++];
			const length = tokenSpan[i++];
			const tsClassification = tokenSpan[i++];

			let tokenModifiers = 0;
			let tokenType = getTokenTypeFromClassification(tsClassification);
			if (tokenType !== undefined) {
				// it's a classification as returned by the typescript-vscode-sh-plugin
				tokenModifiers = getTokenModifierFromClassification(tsClassification);
			} else {
				// typescript-vscode-sh-plugin is not present
				tokenType = tokenTypeMap[tsClassification];
				if (tokenType === undefined) {
					continue;
				}
			}

			// we can use the document's range conversion methods because the result is at the same version as the document
			const startPos = document.positionAt(offset);
			const endPos = document.positionAt(offset + length);

			for (let line = startPos.line; line <= endPos.line; line++) {
				const startCharacter = (line === startPos.line ? startPos.character : 0);
				const endCharacter = (line === endPos.line ? endPos.character : document.lineAt(line).text.length);
				builder.push(line, startCharacter, endCharacter - startCharacter, tokenType, tokenModifiers);
			}
		}
		return builder.build();
	}
}

function waitForDocumentChangesToEnd(document: vscode.TextDocument) {
	let version = document.version;
	return new Promise((s) => {
		let iv = setInterval(_ => {
			if (document.version === version) {
				clearInterval(iv);
				s();
			}
			version = document.version;
		}, 400);
	});
}


// typescript-vscode-sh-plugin encodes type and modifiers in the classification:
// TSClassification = (TokenType + 1) << 8 + TokenModifier

function getTokenTypeFromClassification(tsClassification: number): number | undefined {
	if (tsClassification > TokenEncodingConsts.modifierMask) {
		return (tsClassification >> TokenEncodingConsts.typeOffset) - 1;
	}
	return undefined;
}

function getTokenModifierFromClassification(tsClassification: number) {
	return tsClassification & TokenEncodingConsts.modifierMask;
}

const tokenTypes: string[] = [];
tokenTypes[TokenType.class] = 'class';
tokenTypes[TokenType.enum] = 'enum';
tokenTypes[TokenType.interface] = 'interface';
tokenTypes[TokenType.namespace] = 'namespace';
tokenTypes[TokenType.typeParameter] = 'typeParameter';
tokenTypes[TokenType.type] = 'type';
tokenTypes[TokenType.parameter] = 'parameter';
tokenTypes[TokenType.variable] = 'variable';
tokenTypes[TokenType.enumMember] = 'enumMember';
tokenTypes[TokenType.property] = 'property';
tokenTypes[TokenType.function] = 'function';
tokenTypes[TokenType.member] = 'member';

const tokenModifiers: string[] = [];
tokenModifiers[TokenModifier.async] = 'async';
tokenModifiers[TokenModifier.declaration] = 'declaration';
tokenModifiers[TokenModifier.readonly] = 'readonly';
tokenModifiers[TokenModifier.static] = 'static';
tokenModifiers[TokenModifier.local] = 'local';
tokenModifiers[TokenModifier.defaultLibrary] = 'defaultLibrary';

// make sure token types and modifiers are complete
if (tokenTypes.filter(t => !!t).length !== TokenType._) {
	console.warn('typescript-vscode-sh-plugin has added new tokens types.');
}
if (tokenModifiers.filter(t => !!t).length !== TokenModifier._) {
	console.warn('typescript-vscode-sh-plugin has added new tokens modifiers.');
}

// mapping for the original ExperimentalProtocol.ClassificationType from TypeScript (only used when plugin is not available)
const tokenTypeMap: number[] = [];
tokenTypeMap[ExperimentalProtocol.ClassificationType.className] = TokenType.class;
tokenTypeMap[ExperimentalProtocol.ClassificationType.enumName] = TokenType.enum;
tokenTypeMap[ExperimentalProtocol.ClassificationType.interfaceName] = TokenType.interface;
tokenTypeMap[ExperimentalProtocol.ClassificationType.moduleName] = TokenType.namespace;
tokenTypeMap[ExperimentalProtocol.ClassificationType.typeParameterName] = TokenType.typeParameter;
tokenTypeMap[ExperimentalProtocol.ClassificationType.typeAliasName] = TokenType.type;
tokenTypeMap[ExperimentalProtocol.ClassificationType.parameterName] = TokenType.parameter;

namespace ExperimentalProtocol {

	export interface IExtendedTypeScriptServiceClient {
		execute<K extends keyof ExperimentalProtocol.ExtendedTsServerRequests>(
			command: K,
			args: ExperimentalProtocol.ExtendedTsServerRequests[K][0],
			token: vscode.CancellationToken,
			config?: ExecConfig
		): Promise<ServerResponse.Response<ExperimentalProtocol.ExtendedTsServerRequests[K][1]>>;
	}

	/**
	 * A request to get encoded semantic classifications for a span in the file
	 */
	export interface EncodedSemanticClassificationsRequest extends Proto.FileRequest {
		arguments: EncodedSemanticClassificationsRequestArgs;
	}

	/**
	 * Arguments for EncodedSemanticClassificationsRequest request.
	 */
	export interface EncodedSemanticClassificationsRequestArgs extends Proto.FileRequestArgs {
		/**
		 * Start position of the span.
		 */
		start: number;
		/**
		 * Length of the span.
		 */
		length: number;
	}

	export const enum EndOfLineState {
		None,
		InMultiLineCommentTrivia,
		InSingleQuoteStringLiteral,
		InDoubleQuoteStringLiteral,
		InTemplateHeadOrNoSubstitutionTemplate,
		InTemplateMiddleOrTail,
		InTemplateSubstitutionPosition,
	}

	export const enum ClassificationType {
		comment = 1,
		identifier = 2,
		keyword = 3,
		numericLiteral = 4,
		operator = 5,
		stringLiteral = 6,
		regularExpressionLiteral = 7,
		whiteSpace = 8,
		text = 9,
		punctuation = 10,
		className = 11,
		enumName = 12,
		interfaceName = 13,
		moduleName = 14,
		typeParameterName = 15,
		typeAliasName = 16,
		parameterName = 17,
		docCommentTagName = 18,
		jsxOpenTagName = 19,
		jsxCloseTagName = 20,
		jsxSelfClosingTagName = 21,
		jsxAttribute = 22,
		jsxText = 23,
		jsxAttributeStringLiteralValue = 24,
		bigintLiteral = 25,
	}

	export interface EncodedSemanticClassificationsResponse extends Proto.Response {
		body?: {
			endOfLineState: EndOfLineState;
			spans: number[];
		};
	}

	export interface ExtendedTsServerRequests {
		'encodedSemanticClassifications-full': [ExperimentalProtocol.EncodedSemanticClassificationsRequestArgs, ExperimentalProtocol.EncodedSemanticClassificationsResponse];
	}
}
