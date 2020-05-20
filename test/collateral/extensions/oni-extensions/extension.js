// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');

// this method is called when your extension is activated
// your extension is activated the very first time the command is executed

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
	const showData = (val) => {
		vscode.window.showInformationMessage(JSON.stringify(val));
	};
	
	const disposable0 = vscode.commands.registerCommand("testExtensions.getPackageJSON", (args) => {
		
		const extension = vscode.extensions.getExtension(args);
		showData(extension.packageJSON);
	});

	context.subscriptions.push(disposable0);
}

// this method is called when your extension is deactivated
function deactivate() {}

module.exports = {
	activate,
	deactivate
}
