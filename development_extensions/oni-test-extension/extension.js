// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require("vscode")
const path = require("path");

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


    const pass = () => {
        vscode.window.showInformationMessage("PASS", [])
    };

    const fail = (msg) => {
        vscode.window.showInformationMessage("FAIL: " + msg, [])
    };

    cleanup(
        vscode.commands.registerCommand("oni-test.exthost.validateLogPathIsDirectory", () => {
            (async () => {
                // From docs: https://code.visualstudio.com/api/references/vscode-api#ExtensionContext
                // Given path might not exist - but parent directory will exist.
                const logPath = path.dirname(context.logUri.path);
                const logParentUri = vscode.Uri.file(logPath);
                const statResult = await vscode.workspace.fs.stat(logParentUri);

                if ((statResult.type & vscode.FileType.Directory) == vscode.FileType.Directory) {
                    pass()
                } else {
                    fail("Filetype was: " + statResult.type.toString());
                }
            })();
        }),
    );

    cleanup(
        vscode.commands.registerCommand("oni-test.exthost.runFileSystemTests", () => {
            
            (async () => {
                // From docs: https://code.visualstudio.com/api/references/vscode-api#ExtensionContext
                // Given path might not exist - but parent directory will exist.
                const nonExistentPath = path.join(context.storagePath, "some-non-existent-path");
                const nonExistentUri = vscode.Uri.file(nonExistentPath);
                try {
                    await vscode.workspace.fs.stat(nonExistentUri);
                    fail("#3685: Non-existent path should not have successful stat")
                } catch (exn) {
                    pass()
                }
        })();
    })
    );
}

// this method is called when your extension is deactivated
function deactivate() { }

module.exports = {
    activate,
    deactivate,
}
