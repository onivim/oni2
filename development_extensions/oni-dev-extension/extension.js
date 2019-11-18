// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');
const path = require('path');

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
    let showData = (val) => {
        vscode.window.showInformationMessage(JSON.stringify(val));
    }
    // Create a simple status bar
    let item = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 1000);
    item.text = "Developer";
    item.show();

    // Add a completion provider for oni-dev files
    let provider1 = vscode.languages.registerCompletionItemProvider('oni-dev',
        {
            provideCompletionItems: (document, position, token, context) => {
                return [
                    vscode.CompletionItem('HelloWorld'),
                    vscode.CompletionItem('HelloAgain'),
                ];
            }
        }
    );
    
    let provider2 = vscode.languages.registerCompletionItemProvider('oni-dev',
        {
            provideCompletionItems: (document, position, token, context) => {
                return [
                    vscode.CompletionItem('ReasonML'),
                    vscode.CompletionItem('OCaml'),
                ];
            }
        }
    );

    context.subscriptions.push(provider1);

    const output = vscode.window.createOutputChannel("oni-dev");
    output.appendLine("Hello output channel!");

    const output2 = vscode.window.createOutputChannel("oni-dev2");
    output2.append("Hello output channel!");

    const collection = vscode.languages.createDiagnosticCollection('test');

	// The command has been defined in the package.json file
	// Now provide the implementation of the command with  registerCommand
	// The commandId parameter must match the command field in package.json
	let disposable = vscode.commands.registerCommand('extension.helloWorld', () => {
		// The code you place here will be executed every time your command is executed

		// Display a message box to the user
		vscode.window.showInformationMessage('Hello World!');
	});

    let disposable2 = vscode.workspace.onDidOpenTextDocument((e) => {
        // TODO:
        // Add command / option to toggle this
        // showData({
        //     type: "workspace.onDidOpenTextDocument",
        //     filename: e.fileName,
        // });
    });

    let disposable3 = vscode.workspace.onDidChangeTextDocument((e) => {
        // TODO:
        // Add command / option to toggle this
        // showData({
        //     type: "workspace.onDidChangeTextDocument",
        //     filename: e.document.fileName,
        //     contentChanges: e.contentChanges,
        //     fullText: e.document.getText(),
        // });

		//vscode.window.showInformationMessage('Changed!');
        const document = e.document;
        if (document && path.basename(document.uri.fsPath) == "test.oni-dev") {
           collection.set(document.uri, [{
                code: '', 
                message: "diag 1",
                range: new vscode.Range(new vscode.Position(0, 4), new vscode.Position(0, 10)),
                severity: vscode.DiagnosticSeverity.Error,
                source: '',
                relatedInformation: []
           }]);
        }
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
