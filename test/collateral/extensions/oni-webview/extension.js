// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');

// this method is called when your extension is activated
// your extension is activated the very first time the command is executed

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
  context.subscriptions.push(
    vscode.commands.registerCommand('webview.test.asWebviewUri', () => {
      // Create and show panel
      const panel = vscode.window.createWebviewPanel(
        'test-webview',
        'Test Webview',
        vscode.ViewColumn.One,
        {}
      );
      const onDiskPath = vscode.Uri.file(
        context.extensionPath
      );

      // And set its HTML content
	  // Onivim doesn't support webviews, yet, but we shouldn't crash in trying to interact with one...
      panel.webview.html = "<html />";

	  const myUri = panel.webview.asWebviewUri(onDiskPath);

	  vscode.window.showInformationMessage("Success: " + myUri.toString());
    })
  );
}

// this method is called when your extension is deactivated
function deactivate() {}

module.exports = {
	activate,
	deactivate
}
