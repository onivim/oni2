// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require("vscode")

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
    let cleanup = (disposable) => context.subscriptions.push(disposable)

    cleanup(
        vscode.commands.registerCommand("oni-test.showMessage", (message) => {
            vscode.window.showInformationMessage(message, [])
        }),
    );
}

// this method is called when your extension is deactivated
function deactivate() { }

module.exports = {
    activate,
    deactivate,
}
