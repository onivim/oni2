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

	const disposable0 = vscode.workspace.onDidOpenTextDocument((e) => {
		showData({
			type: "workspace.onDidOpenTextDocument",
			filename: e.fileName,
			fullText: e.getText(),
		});
	});
	const disposable1 = vscode.workspace.onDidCloseTextDocument((e) => {
		showData({
			type: "workspace.onDidCloseTextDocument",
			filename: e.fileName,
		});
	});
	
	const disposable2 = vscode.workspace.onDidChangeTextDocument((e) => {
		showData({
			type: "workspace.onDidChangeTextDocument",
			filename: e.fileName,
			fullText: e.getText(),
		});
	});

	context.subscriptions.push(disposable0);	
	context.subscriptions.push(disposable1);
	context.subscriptions.push(disposable2);
}

// this method is called when your extension is deactivated
function deactivate() {}

module.exports = {
	activate,
	deactivate
}
