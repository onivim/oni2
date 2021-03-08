// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require("vscode")
const path = require("path")
const os = require("os")

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
    let showData = (val) => {
        vscode.window.showInformationMessage(JSON.stringify(val))
    }
    // Create a simple status bar
    let item = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 1000)
    item.color = new vscode.ThemeColor("foreground")
    // TODO: Bring back
    // item.backgroundColor = new vscode.ThemeColor("statusBarItem.errorBackground")
    item.command = "developer.oni.statusBarClicked"
    item.text = "$(wrench) Developer"
    item.tooltip = "Hello from oni-dev-extension!"
    item.show()

    let cleanup = (disposable) => context.subscriptions.push(disposable)

    cleanup(
        vscode.commands.registerCommand("developer.oni.statusBarClicked", () => {
            vscode.window.showWarningMessage("You clicked developer", [])
        }),
    )

    cleanup(
        vscode.commands.registerCommand("_test.findFiles", (count) => {
            vscode.workspace.findFiles(
                "*.json", undefined, count
            ).then((results) => {
                vscode.window.showInformationMessage("success:" + results.length);
                // vscode.window.showInformationMessage("success:1");
            })
        }),
    )

    cleanup(
        vscode.commands.registerCommand("developer.oni.hideStatusBar", () => {
            item.hide();
        }),
    )

    cleanup(
        vscode.commands.registerCommand("developer.oni.logBufferUpdates", () => {
            vscode.window.showInformationMessage("Logging buffer updates!");
            vscode.workspace.onDidChangeTextDocument((e) => {
                console.log({
                    type: "workspace.onDidChangeTextDocument",
                    filename: e.document.fileName,
                    version: e.document.version,
                    contentChanges: e.contentChanges,
                    fullText: e.document.getText(),
                });
            });
        }),
    )

    cleanup(
        vscode.commands.registerCommand("developer.oni.tryOpenDocument", () => {
            vscode.workspace.openTextDocument(vscode.Uri.file("package.json"))
                .then((document) => {
                    let text = document.getText();
                    vscode.window.showInformationMessage("Got text document: " + text);
                }, err => {
                    vscode.window.showErrorMessage("Failed to get text document: " + err.toString());
                })
        }),
    )

    cleanup(
        vscode.commands.registerCommand("developer.oni.showStatusBar", () => {
            item.show();
        }),
    )

    cleanup(
        vscode.languages.registerDefinitionProvider("oni-dev", {
            provideDefinition: (document, _position, _token) => {
                return new vscode.Location(document.uri, new vscode.Position(0, 0))
            },
        }),
    )

    // Add a completion provider for oni-dev files
    cleanup(
        vscode.languages.registerCompletionItemProvider("oni-dev", {
            provideCompletionItems: (document, position, token, context) => {
                return [vscode.CompletionItem("HelloWorld"), vscode.CompletionItem("HelloAgain")]
            },
            resolveCompletionItem: (completionItem, token) => {
                completionItem.documentation = "RESOLVED documentation:  " + completionItem.label;
                completionItem.detail = "RESOLVED detail: " + completionItem.label;
                return completionItem;
            }
        }),
    )

    cleanup(
        vscode.languages.registerCompletionItemProvider("oni-dev", {
            provideCompletionItems: (document, position, token, context) => {
                const itemWithAdditionalEdit = vscode.CompletionItem("ReasonML1");
                itemWithAdditionalEdit.detail = "(Inserts line at top too)";
                const range0 = new vscode.Range(0, 0, 0, 0);
                const edit0 = new vscode.TextEdit(range0, "Insert line up top!\n");
                itemWithAdditionalEdit.additionalTextEdits = [
                    edit0
                ];

                const itemWithAdditionalEditAfter = vscode.CompletionItem("OCaml");
                itemWithAdditionalEditAfter.detail = "(Inserts line at line 10, too)";
                const range1 = new vscode.Range(11, 0, 11, 0);
                const edit1 = new vscode.TextEdit(range1, "Insert line at line 10\n");
                itemWithAdditionalEditAfter.additionalTextEdits = [
                    edit1
                ];
                return [itemWithAdditionalEdit, itemWithAdditionalEditAfter];
            },
        }),
    )

    // SLOW completion provider
    cleanup(
        vscode.languages.registerCompletionItemProvider("oni-dev", {
            provideCompletionItems: (document, position, token, context) => {
                return new Promise((resolve) => {
                    setTimeout(() => {
                        resolve([
                            vscode.CompletionItem("ReasonML0"),
                            vscode.CompletionItem("OCaml0"),
                            vscode.CompletionItem("ReasonML2"),
                            vscode.CompletionItem("OCaml2"),
                        ])
                    }, 500);
                });
            },
        }),
    )

    // INCOMPLETE completion provider
    cleanup(
        vscode.languages.registerCompletionItemProvider("oni-dev", {
            provideCompletionItems: (document, position, token, context) => {
                const items = [
                    vscode.CompletionItem("Incomplete" + Date.now().toString()),
                ];
                return {
                    isIncomplete: true,
                    items,
                };
            },
        }),
    )

    const output = vscode.window.createOutputChannel("oni-dev")
    output.appendLine("Hello output channel!")

    const output2 = vscode.window.createOutputChannel("oni-dev2")
    output2.append("Hello output channel!")

    const collection = vscode.languages.createDiagnosticCollection("test")

    let latestText = ""

    cleanup(
        vscode.workspace.onDidOpenTextDocument((document) => {
            // TODO:
            // Add command / option to toggle this
            // showData({
            //     type: "workspace.onDidOpenTextDocument",
            //     filename: e.fileName,
            // });

            if (document) {
                latestText = document.getText().split(os.EOL).join("|")
            }
        }),
    )

    cleanup(
        vscode.workspace.onDidChangeTextDocument((e) => {
            // TODO:
            // Add command / option to toggle this
            // showData({
            //     type: "workspace.onDidChangeTextDocument",
            //     filename: e.document.fileName,
            //     contentChanges: e.contentChanges,
            //     fullText: e.document.getText(),
            // });

            if (e.document) {
                latestText = e.document.getText().split(os.EOL).join("|")
            }

            //vscode.window.showInformationMessage('Changed!');
            const document = e.document
            if (document && path.basename(document.uri.fsPath) == "test.oni-dev") {
                collection.set(document.uri, [
                    {
                        code: "",
                        message: "diag 1",
                        range: new vscode.Range(
                            new vscode.Position(0, 4),
                            new vscode.Position(0, 10),
                        ),
                        severity: vscode.DiagnosticSeverity.Error,
                        source: "",
                        relatedInformation: [],
                    },
                ])
            }
        }),
    )

    cleanup(
        vscode.commands.registerCommand("developer.oni.showChoiceMessage", () => {
            vscode.window.showInformationMessage(
                //`Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?`
                "Hello!"
                , "Option 1", "Option 2", "Option 3").then(
                    (result) => {
                        vscode.window.showInformationMessage("You picked: " + result)
                    },
                    (err) => {
                        vscode.window.showInformationMessage("Cancelled: " + err)
                    },
                )
        }),
    )

    cleanup(
        vscode.commands.registerCommand("developer.oni.showWarning", () => {
            vscode.window.showWarningMessage("This is a warning")
        }),
    )

    cleanup(
        vscode.commands.registerCommand("developer.oni.showError", () => {
            vscode.window.showErrorMessage("Hello, this is error")
        }),
    )

    cleanup(
        vscode.commands.registerCommand("developer.oni.showWorkspaceRootPath", () => {
            vscode.window.showInformationMessage("Workspace rootPath: " + vscode.workspace.rootPath)
            vscode.window.showInformationMessage("Workspace storagePath: " + context.storagePath)
        }),
    )

    cleanup(
        vscode.commands.registerCommand("developer.oni.showWorkspaceFolders", () => {
            vscode.window.showInformationMessage(
                "Workspace folders: " + JSON.stringify(vscode.workspace.workspaceFolders),
            )
        }),
    )

    // Helper command to show buffer text
    // This helps us create a test case to validate buffer manipulations
    cleanup(
        vscode.commands.registerCommand("developer.oni.getBufferText", () => {
            vscode.window.showInformationMessage("fulltext:" + latestText)
        }),
    )

    cleanup(
        vscode.commands.registerCommand("developer.oni.showActiveEditor", () => {
            vscode.window.showInformationMessage(
                "Active editor: " + JSON.stringify(vscode.window.activeTextEditor),
            )
        }),
    )

    cleanup(
        vscode.commands.registerCommand("developer.oni.showVisibleTextEditors", () => {
            vscode.window.showInformationMessage(
                "Visible editors: " + JSON.stringify(vscode.window.visibleTextEditors),
            )
        }),
    )

    cleanup(
        vscode.commands.registerCommand("developer.oni.showQuickPick", () => {
            vscode.window.showQuickPick(["Item 1", "Item 2", "Item 3"]).then(
                (selectedItem) => {
                    vscode.window.showInformationMessage(
                        "Quick pick item selected: " + selectedItem,
                    )
                },
                (err) => {
                    vscode.window.showErrorMessage("Quick pick cancelled: " + err)
                },
            )
        }),
    )

    const getValue = (str) => {
        return str + ":" + new Date().getTime().toString()
    }

    cleanup(
        vscode.commands.registerCommand("developer.oni.setGlobalMemento", () => {
            const valueToSet = getValue("oni-dev.global")
            context.globalState.update("oni-dev.global", valueToSet)
            vscode.window.showInformationMessage("Set global value: " + valueToSet)
        }),
    )

    cleanup(
        vscode.commands.registerCommand("developer.oni.setWorkspaceMemento", () => {
            const valueToSet = getValue("oni-dev.workspace")
            context.globalState.update("oni-dev.workspace", valueToSet)
            vscode.window.showInformationMessage("Set workspace value: " + valueToSet)
        }),
    )

    cleanup(
        vscode.commands.registerCommand("developer.oni.getGlobalMemento", () => {
            const value = context.globalState.get("oni-dev.global")
            vscode.window.showInformationMessage("Got global value: " + value)
        }),
    )

    cleanup(
        vscode.commands.registerCommand("developer.oni.getWorkspaceMemento", () => {
            context.globalState.get("oni-dev.workspace").then((value) => {
                vscode.window.showInformationMessage("Got workspace value: " + value)
            })
        }),
    )

    if (vscode.workspace.rootPath) {
        function createResourceUri(relativePath) {
            const absolutePath = path.join(vscode.workspace.rootPath, relativePath)
            return vscode.Uri.file(absolutePath)
        }

        // Test SCM

        const testSCM = vscode.scm.createSourceControl("test", "Test")

        const index = testSCM.createResourceGroup("index", "Index")
        index.resourceStates = [
            { resourceUri: createResourceUri("README.md") },
            { resourceUri: createResourceUri("src/test/api.ts") },
        ]

        const workingTree = testSCM.createResourceGroup("workingTree", "Changes")
        workingTree.resourceStates = [
            { resourceUri: createResourceUri(".travis.yml") },
            { resourceUri: createResourceUri("README.md") },
        ]

        testSCM.count = 13

        testSCM.quickDiffProvider = {
            provideOriginalResource: (uri, _token) => {
                return vscode.Uri.file("README.md.old")
            },
        }

        testSCM.dispose()
    };

    // Text Document Content Provider

    const textContentProvider = {
        provideTextDocumentContent: (uri) => {
            console.error("CONTENT!")
            return "Hello. This is content."
        },
    }
    let disposable = vscode.workspace.registerTextDocumentContentProvider(
        "foo",
        textContentProvider,
    )
    disposable.dispose()

    console.log("Storage path available: " + context.storagePath);

    // Configuration

    const rlsLocation = vscode.workspace.getConfiguration().get("reason_language_server.location")
    console.error("Configured RLS location: ", rlsLocation)

    const editorFontFamily = vscode.workspace.getConfiguration().get("editor.fontFamily")
    console.error("Editor Font Family: ", editorFontFamily)

    const testSetting = vscode.workspace.getConfiguration().get("developer.oni.test")
    console.error("Test setting: ", testSetting)

    vscode.workspace.onDidChangeConfiguration((event) => {
        if (event.affectsConfiguration("developer.oni.test")) {
            const setting = vscode.workspace.getConfiguration().get("developer.oni.test")
            vscode.window.showInformationMessage("Setting changed: " + setting)
        }
    })
}

// this method is called when your extension is deactivated
function deactivate() { }

module.exports = {
    activate,
    deactivate,
}
