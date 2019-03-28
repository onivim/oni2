// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');

// this method is called when your extension is activated
// your extension is activated the very first time the command is executed

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
	// Use the console to output diagnostic information (console.log) and errors (console.error)
	// This line of code will only be executed once when your extension is activated

	// console.log('Congratulations, your extension "helloworld-minimal-sample" is now active!');

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

    let disposable3 = vscode.workspace.onDidCloseTextDocument((e) => {
        showData({
            type: "workspace.onDidCloseTextDocument",
            filename: e.fileName,
        });
    });

	context.subscriptions.push(disposable);
    context.subscriptions.push(disposable2);
    context.subscriptions.push(disposable3);
}

// this method is called when your extension is deactivated
function deactivate() {}

module.exports = {
	activate,
	deactivate
}
