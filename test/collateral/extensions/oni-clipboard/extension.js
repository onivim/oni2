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

	const sendSuccess = (text) => {
		vscode.env.clipboard.writeText("success:" + text);
	};

	const sendFailure = (err) => {
		vscode.env.clipboard.writeText("failed");
		showData({
			type: "clipboard.getText",
			result: "failure",
			error: err.toString(),
		});
	};

	const disposable0 = vscode.commands.registerCommand("clipboard.getText", () => {
		vscode.env.clipboard.readText().then(
			sendSuccess,
			sendFailure
		);
	});
	const disposable1 = vscode.commands.registerCommand("clipboard.setText", (arg) => {
		vscode.env.clipboard.writeText(arg);
	});
	
	context.subscriptions.push(disposable0);	
	context.subscriptions.push(disposable1);
}

// this method is called when your extension is deactivated
function deactivate() {}

module.exports = {
	activate,
	deactivate
}
