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
    item.command = "open sesame";
    item.color = new vscode.ThemeColor("foreground");
    item.command = "oni.developer.statusBarClicked";
    item.text = "Developer";
    item.show();

    let cleanup = (disposable) => context.subscriptions.push(disposable);
    
    cleanup(vscode.commands.registerCommand('developer.oni.statusBarClicked', () => {
        vscode.window.showWarningMessage('You clicked developer');
    }));

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

    cleanup(vscode.commands.registerCommand('developer.oni.showWarning', () => {
        vscode.window.showWarningMessage('This is a warning');
    }));

    cleanup(vscode.commands.registerCommand('developer.oni.showError', () => {
        vscode.window.showErrorMessage('Hello, this is error');
    }));
    
    cleanup(vscode.commands.registerCommand('developer.oni.showWorkspaceRootPath', () => {
        vscode.window.showInformationMessage("Workspace rootPath: " + vscode.workspace.rootPath);
    }));
    
    cleanup(vscode.commands.registerCommand('developer.oni.showWorkspaceFolders', () => {
        vscode.window.showInformationMessage("Workspace folders: " +
            JSON.stringify(vscode.workspace.workspaceFolders));
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

    // Test SCM

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

    testSCM.count = 13;

    testSCM.quickDiffProvider = {
        provideOriginalResource: (uri, _token) => {
            return vscode.Uri.file("README.md.old");
        }
    };

    testSCM.dispose();

    // Text Document Content Provider

    const textContentProvider = {
        provideTextDocumentContent: (uri) => {
            console.error("CONTENT!");
            return "Hello. This is content.";
        }
    };
    let disposable = vscode.workspace.registerTextDocumentContentProvider('foo', textContentProvider)
    disposable.dispose();

    // Configuration

    const rlsLocation = vscode.workspace.getConfiguration().get("reason_language_server.location");
    console.error("Configured RLS location: ", rlsLocation);

    const editorFontFamily = vscode.workspace.getConfiguration().get("editor.fontFamily");
    console.error("Editor Font Family: ", editorFontFamily);

    const testSetting = vscode.workspace.getConfiguration().get("developer.oni.test");
    console.error("Test setting: ", testSetting);

    vscode.workspace.onDidChangeConfiguration(event => {
      if (event.affectsConfiguration("developer.oni.test")) {
        const setting = vscode.workspace.getConfiguration().get("developer.oni.test");
        vscode.window.showInformationMessage("Setting changed: " + setting);
      }
    });
}

// this method is called when your extension is deactivated
function deactivate() {}

module.exports = {
    activate,
    deactivate
}
