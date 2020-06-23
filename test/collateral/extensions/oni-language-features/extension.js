// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');

// this method is called when your extension is activated
// your extension is activated the very first time the command is executed

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {

	const codeLensProvider = {
		provideCodeLenses: (document, token) => {
			const range1 = new vscode.Range(1, 1, 1, 1);
			const range2 = new vscode.Range(2, 2, 2, 2);
			const command1 = {
				command: "codelens.command1",
				title: "codelens: command1"
			};
			const command2 = {
				command: "codelens.command2",
				title: "codelens: command2"
			};
			return [
				new vscode.CodeLens(range1, command1),
				new vscode.CodeLens(range2, command2),
			];
		}
	};

	const completionProvider = {
		provideCompletionItems: (document, position, token, context) => {
			return [{
				label: "item1",
			}, {
				label: "item2"
			}]
		}
	};

	const documentHighlightProvider = {
		provideDocumentHighlights: (document, position, token) => {
			return [
				new vscode.DocumentHighlight(new vscode.Range(1, 2, 3, 4), vscode.DocumentHighlightKind.Text),
				new vscode.DocumentHighlight(new vscode.Range(5, 6, 7, 8), vscode.DocumentHighlightKind.Text),
			]
		}
	};

	const hardcodedLocationProvider = (pos) => (document, _position, _token) => {
		return new vscode.Location(document.uri, pos);
	};

	const commonFormat = (str) => {
		return [
			new vscode.TextEdit(
				new vscode.Range(1, 2, 3, 4),
				str,
			)
		];
	};

	const asYouTypeFormat = (_document, _position, _ch, _options, _token) => {
		return commonFormat("as-you-type");
	};

	const rangeFormat = (_document, _range, _options, _token) => {
		return commonFormat("range");
	};

	const format = (_document, _options, _token) => {
		return commonFormat("document");
	};

	const documentFormattingProvider = {
		provideDocumentFormattingEdits: format,
		provideDocumentRangeFormattingEdits: rangeFormat,
		provideOnTypeFormattingEdits: asYouTypeFormat,
	};

	const signatureHelpProvider = {
		provideSignatureHelp: (_document, _position, _token, _context) => {
			const signature1 = new vscode.SignatureInformation("signature 1", "signature 1 documentation");
			signature1.parameters = [
				new vscode.ParameterInformation("gnat", "parameter 1 documentation")
			];

			// Signature Help
			return {
				activeParameter: 0,
				activeSignature: 0,
				signatures: [
					signature1,
				]
			};
		}
	};

	const referenceProvider =  {
		provideReferences: (document, _position, _token) => {
			return [
				new vscode.Location(document.uri, new vscode.Range(1, 2, 3, 4))
			]
		}
	};

	const hoverProvider = {
		provideHover: (_document, _position, _token) => {
			return {
				contents: ['Hover Content'],
			}
		}
	};

	const definitionProvider = {
		provideDefinition: hardcodedLocationProvider(new vscode.Position(0, 0))
	};
	const declarationProvider = {
		provideDeclaration: hardcodedLocationProvider(new vscode.Position(1, 1))
	};
	const typeDefinitionProvider= {
		provideTypeDefinition: hardcodedLocationProvider(new vscode.Position(2, 2))
	};

	const implementationProvider = {
		provideImplementation: hardcodedLocationProvider(new vscode.Position(3, 3))
	};

	const documentSymbolProvider = {
		provideDocumentSymbols: (document, _token) => {
			const position = new vscode.Position(0, 0);
			return [
				new vscode.SymbolInformation("symbol1", vscode.SymbolKind.File, "symbol1-container", new vscode.Location(document.uri, position)),
				new vscode.SymbolInformation("symbol2", vscode.SymbolKind.TypeParameter, "symbol2-container", new vscode.Location(document.uri, position)),
			];
		},
	};

	[
		vscode.languages.registerCodeLensProvider("plaintext", codeLensProvider),
		vscode.languages.registerCompletionItemProvider("plaintext", completionProvider, ["."]),
		vscode.languages.registerDefinitionProvider("plaintext", definitionProvider),
		vscode.languages.registerDeclarationProvider("plaintext", declarationProvider),
		vscode.languages.registerTypeDefinitionProvider("plaintext", typeDefinitionProvider),
		vscode.languages.registerImplementationProvider("plaintext", implementationProvider),
		vscode.languages.registerDocumentHighlightProvider("plaintext", documentHighlightProvider),
		vscode.languages.registerReferenceProvider("plaintext", referenceProvider),
		vscode.languages.registerDocumentSymbolProvider("plaintext", documentSymbolProvider),
		vscode.languages.registerSignatureHelpProvider("plaintext", signatureHelpProvider, {
			triggerCharacters: ["("],
			retriggerCharacters: [","]
		}),
		vscode.languages.registerHoverProvider("plaintext", hoverProvider),
		vscode.languages.registerOnTypeFormattingEditProvider("plaintext", documentFormattingProvider, "{"),
		vscode.languages.registerDocumentFormattingEditProvider("plaintext", documentFormattingProvider, "{"),
		vscode.languages.registerDocumentRangeFormattingEditProvider("plaintext", documentFormattingProvider, "{")
	].forEach(subscription => context.subscriptions.push(subscription));
}

// this method is called when your extension is deactivated
function deactivate() {}

module.exports = {
	activate,
	deactivate
}
