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

	const disposable0 = vscode.commands.registerCommand("config.show", (args) => {
	
		const config = vscode.workspace.getConfiguration();
		showData({
			eventType: "config.value",
			result: config.get(args),
		});


		/*const config = vscode.workspace.getConfiguration("foo");
		const values = config.inspect("bar");

		showData({
			type: "config.inspect",
			result: JSON.stringify(values)
		});*/
	});

	const disposable1 = vscode.workspace.onDidChangeConfiguration((evt) => {
		console.log(evt);
		/*showData({
			type: "config.changed",
			evt: evt,
		});*/
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
