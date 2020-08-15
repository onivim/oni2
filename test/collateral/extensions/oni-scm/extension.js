// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');

// this method is called when your extension is activated
// your extension is activated the very first time the command is executed

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {

    const testSCM = vscode.scm.createSourceControl('test', 'Test');

	const resourceGroup = testSCM.createResourceGroup('resourceGroup', 'resourceGroup');
	resourceGroup.resourceStates = [];

	testSCM.quickDiffProvider = {
		provideOriginalResource: (uri, _token) => {
			// Test round-trip
			return uri;
		}
	};
	
	const disposable0 = vscode.commands.registerCommand("testSCM.dispose", (_args) => {
		testSCM.dispose();
	});

	context.subscriptions.push(disposable0);
}

// this method is called when your extension is deactivated
function deactivate() {}

module.exports = {
	activate,
	deactivate
}
