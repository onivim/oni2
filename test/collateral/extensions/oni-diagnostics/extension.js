// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');

// this method is called when your extension is activated
// your extension is activated the very first time the command is executed

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
	let diags = vscode.languages.createDiagnosticCollection("diags");
	let uri = vscode.Uri.file("test1.txt");
	let diagnostic = new vscode.Diagnostic(
		new vscode.Range(1, 1, 2, 2),
		"test diagnostic"
	);
	diags.set(uri, [diagnostic]);
	diags.clear();
}

// this method is called when your extension is deactivated
function deactivate() {}

module.exports = {
	activate,
	deactivate
}
