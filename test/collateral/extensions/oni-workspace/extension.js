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

	const disposable0 = vscode.commands.registerCommand("workspace.showActive", () => {
		showData({
			type: "workspace.show",
			workspacePath: vscode.workspace.rootPath,
			added: 0,
			removed: 0,
		});
	});
	const disposable1 = vscode.workspace.onDidChangeWorkspaceFolders((wf) => {
		showData({
			type: "workspace.changed",
			workspacePath: vscode.workspace.rootPath,
			added: wf.added.length,
			removed: wf.removed.length,
		});
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
