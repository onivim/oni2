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
	
	const disposable0 = vscode.commands.registerCommand("fs.stat", (args) => {
		showData("Command executed: " + JSON.stringify(args));


		vscode.workspace.fs.stat(vscode.Uri.file("test.txt"))
		.then((statResult) => {
			showData({
				result: "success",
				payload: statResult,
			})
		}, (err) => {
			showData({
				result: "error",
				error: err,
			})
		});
	});

	context.subscriptions.push(disposable0);
}

// this method is called when your extension is deactivated
function deactivate() {}

module.exports = {
	activate,
	deactivate
}
