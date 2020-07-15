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


		//vscode.workspace.fs.writeFile(vscode.Uri.file("write.test"), new Uint8Array(1234), { create: true, overwrite: true});

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

	const disposable1 = vscode.commands.registerCommand("fs.write", (args) => {
		showData("Command executed: " + JSON.stringify(args));


		const uint8Array = new Uint8Array(1234);
		vscode.workspace.fs.writeFile(vscode.Uri.file("write.test"), uint8Array, { create: true, overwrite: true});
	});

	const disposable2 = vscode.commands.registerCommand("fs.read", (args) => {
		showData("Command executed: " + JSON.stringify(args));

		vscode.workspace.fs.readFile(vscode.Uri.file("read.test"))
		.then((readResult) => {
			showData({
				result: "success",
				payload: readResult.toString(),
			})
		}, (err) => {
			showData({
				result: "error",
				payload: err,
			})
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
