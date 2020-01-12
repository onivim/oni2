// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');
const path = require('path');
const os = require('os');

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

    let cleanup = (disposable) => context.subscriptions.push(disposable);

    cleanup(vscode.languages.registerDefinitionProvider('oni-dev', {
        provideDefinition: (document, _position, _token) => {
             return new vscode.Location(document.uri, new vscode.Position(0, 0));
        }
    }));

    // Add a completion provider for oni-dev files
    cleanup(vscode.languages.registerCompletionItemProvider('oni-dev',
        {
            provideCompletionItems: (document, position, token, context) => {
                return [
                    vscode.CompletionItem('HelloWorld'),
                    vscode.CompletionItem('HelloAgain'),
                ];
            }
        }
    ));
    
    cleanup(vscode.languages.registerCompletionItemProvider('oni-dev',
        {
            provideCompletionItems: (document, position, token, context) => {
                return [
                    vscode.CompletionItem('ReasonML'),
                    vscode.CompletionItem('OCaml'),
                ];
            }
        }
    ));

    const output = vscode.window.createOutputChannel("oni-dev");
    output.appendLine("Hello output channel!");

    const output2 = vscode.window.createOutputChannel("oni-dev2");
    output2.append("Hello output channel!");

    const collection = vscode.languages.createDiagnosticCollection('test');

    cleanup(vscode.workspace.onDidOpenTextDocument((e) => {
        // TODO:
        // Add command / option to toggle this
        // showData({
        //     type: "workspace.onDidOpenTextDocument",
        //     filename: e.fileName,
        // });
    }));

    let latestText = "";

    cleanup(vscode.workspace.onDidChangeTextDocument((e) => {
        // TODO:
        // Add command / option to toggle this
        // showData({
        //     type: "workspace.onDidChangeTextDocument",
        //     filename: e.document.fileName,
        //     contentChanges: e.contentChanges,
        //     fullText: e.document.getText(),
        // });

        if (e.document) {
           latestText = e.document.getText().split(os.EOL).join("|");
        }

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
    }));

	// The command has been defined in the package.json file
	// Now provide the implementation of the command with  registerCommand
	// The commandId parameter must match the command field in package.json
	cleanup(vscode.commands.registerCommand('developer.oni.showNotification', () => {
		// The code you place here will be executed every time your command is executed

		// Display a message box to the user
		vscode.window.showInformationMessage('Hello from extension!');
	}));
    
	cleanup(vscode.commands.registerCommand('developer.oni.showWorkspaceRootPath', () => {
		// Display a message box to the user
		vscode.window.showInformationMessage("rootPath: " + vscode.workspace.rootPath);
	}));

    // Helper command to show buffer text
    // This helps us create a test case to validate buffer manipulations
	cleanup(vscode.commands.registerCommand('developer.oni.getBufferText', () => {
		vscode.window.showInformationMessage("fulltext:" + latestText);
    }));
    
    function createResourceUri(relativePath) {
        const absolutePath = path.join(vscode.workspace.rootPath, relativePath);
        return vscode.Uri.file(absolutePath);
    }

    const testSCM = vscode.scm.createSourceControl('test', 'Test');

    const index = testSCM.createResourceGroup('index', 'Index');
    index.resourceStates = [
        { resourceUri: createResourceUri('README.md') },
        { resourceUri: createResourceUri('src/test/api.ts') }
    ];

    const workingTree = testSCM.createResourceGroup('workingTree', 'Changes');
    workingTree.resourceStates = [
        { resourceUri: createResourceUri('.travis.yml') },
        { resourceUri: createResourceUri('README.md') }
    ];

    testSCM.quickDiffProvider = {
        provideOriginalResource: (uri, _token) => {
            console.log("1");
            Uri.file("/home/glennsl/elm-package.json");
            console.log("2");
        }
    };
}

// this method is called when your extension is deactivated
function deactivate() {}

module.exports = {
	activate,
	deactivate
}
