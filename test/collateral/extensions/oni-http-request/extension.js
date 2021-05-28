// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');
const https = require('https');

// this method is called when your extension is activated
// your extension is activated the very first time the command is executed

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
	context.subscriptions.push(
		// Execute a 2-parameter request against an https object
		vscode.commands.registerCommand('http.request2', (_args) => {
			const req = https.request("https://httpbin.org/json", (res) => {
				if (res.statusCode == 200) {
					vscode.window.showInformationMessage('success');
				} else {
					vscode.window.showErrorMessage('Unexpected status code: ' + res.statusCode)
				}
			});

			req.on('error', (e) => {
				vscode.window.showErrorMessage('Error: ' + e)
			});

			req.end();

		})
	);

	context.subscriptions.push(
		// Execute a 3-parameter request against an https object
		// This exercises the same failure that was causing #2981 -
		// the [agent-base] NPM package overriding the https.request,
		// only handling 2-param arguments.
		vscode.commands.registerCommand('http.request3', (_args) => {
			const req = https.request("https://httpbin.org/json", { agent: false }, (res) => {
				if (res.statusCode == 200) {
					vscode.window.showInformationMessage('success');
				} else {
					vscode.window.showErrorMessage('Unexpected status code: ' + res.statusCode)
				}
			});

			req.on('error', (e) => {
				vscode.window.showErrorMessage('Error: ' + e)
			});

			req.end();

		})
	);
}

// this method is called when your extension is deactivated
function deactivate() {}

module.exports = {
	activate,
	deactivate
}
