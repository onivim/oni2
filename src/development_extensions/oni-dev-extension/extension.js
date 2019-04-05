// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
    let showData = (val) => {
        vscode.window.showInformationMessage(JSON.stringify(val));
    }

	// The command has been defined in the package.json file
	// Now provide the implementation of the command with  registerCommand
	// The commandId parameter must match the command field in package.json
	let disposable = vscode.commands.registerCommand('extension.helloWorld', () => {
		// The code you place here will be executed every time your command is executed

		// Display a message box to the user
		vscode.window.showInformationMessage('Hello World!');
	});

    let disposable2 = vscode.workspace.onDidOpenTextDocument((e) => {
        showData({
            type: "workspace.onDidOpenTextDocument",
            filename: e.fileName,
        });
    });

    // Create a simple status bar
    let item = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 1000);
    item.text = "Developer";
    item.show();

	context.subscriptions.push(disposable);
	context.subscriptions.push(disposable2);
}

// this method is called when your extension is deactivated
function deactivate() {}

module.exports = {
	activate,
	deactivate
}
