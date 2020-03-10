/* --------------------------------------------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 * ------------------------------------------------------------------------------------------ */
'use strict';
const vscode = require("vscode");
const {Uri} = vscode;
const {LanguageClient, RevealOutputChannelOn} = require("vscode-languageclient");
const {TextDocument} = require('vscode-languageserver-types');
const path = require('path')
const fs = require('fs')
const cp = require('child_process');

const validatePath = (pathToValidate, context) => {
    if (!pathToValidate) {
        return null;
    }

    if (!path.isAbsolute(pathToValidate)) {
        pathToValidate = path.join(vscode.workspace.rootPath, pathToValidate);
    }

    if (!fs.existsSync(pathToValidate)) {
        // TODO: Log?
        return null;
    }

    return pathToValidate;
};

const isEsyProject = () => {
    // We assume if there is an OPAM file, this must be a native project.
    return true;
};

const isEsyAvailable = (projectPath) => {
    try {
        cp.execSync("esy --version", { cwd: projectPath });
        return true;
    } catch (ex) {
        return false;
    }
};

const addExe = (filePath) => {
    if (process.platform == "win32") {
        return filePath + ".exe"
    } else {
        return filePath
    }
}

const getOcamlLspPath = (projectPath) => {
    try {
        let ocamlLspDirectory = cp.execSync("esy -q sh -c 'echo #{@opam/ocaml-lsp-server.bin}'", { cwd: projectPath })
       .toString()
       .trim();
        return path.join(ocamlLspDirectory, addExe("ocamllsp"));
    } catch (ex) {
        return null;
    }
};

const getLocation = (_context) => {
    //let binaryLocation = vscode.workspace.getConfiguration('reason_language_server').get('location')

    //binaryLocation = "/Users/bryphe/.esy/3__________________________________________________________________/i/opam__s__ocaml_lsp_server-111372b5/bin/ocamllsp";
    //vscode.window.showErrorMessage('Reason Language Server not found! You specified ' + binaryLocation);

    let rlsBinaryLocation = validatePath(vscode.workspace.getConfiguration('reason_language_server').get('location'));

    const projectPath = vscode.workspace.rootPath;
    if (isEsyAvailable(projectPath) && isEsyProject(projectPath)) {
        // Let's see if ocaml-lsp-server is available
    vscode.window.showErrorMessage("ESY AVAILABLE");
        const ocamlLspPath = getOcamlLspPath(projectPath);
        if (ocamlLspPath) {
            vscode.window.showErrorMessage('Got ocaml LSP binary: ' + ocamlLspPath);
            return ocamlLspPath;
        } else {
            return rlsBinaryLocation
        }
    }

    //vscode.window.showErrorMessage('Using language server:' + rlsBinaryLocation);
    return rlsBinaryLocation;
}

const shouldReload = () => vscode.workspace.getConfiguration('reason_language_server').get('reloadOnChange')

/**
 * Taken from https://github.com/rust-lang/rls-vscode/blob/master/src/extension.ts
 * 
 * Sets up additional language configuration that's impossible to do via a
 * separate language-configuration.json file. See [1] for more information.
 *
 * [1]: https://github.com/Microsoft/vscode/issues/11514#issuecomment-244707076
 */
function configureLanguage() {
  return vscode.languages.setLanguageConfiguration('reason', {
    onEnterRules: [
      {
        // Begins an auto-closed multi-line comment (standard or parent doc)
        // e.g. /** | */ or /*! | */
        beforeText: /^\s*\/\*(\*|\!)(?!\/)([^\*]|\*(?!\/))*$/,
        afterText: /^\s*\*\/$/,
        action: { indentAction: vscode.IndentAction.IndentOutdent, appendText: ' * ' },
      },
      {
        // Begins a multi-line comment (standard or parent doc)
        // e.g. /** ...| or /*! ...|
        beforeText: /^\s*\/\*(\*|\!)(?!\/)([^\*]|\*(?!\/))*$/,
        action: { indentAction: vscode.IndentAction.None, appendText: ' * ' },
      },
      {
        // Continues a multi-line comment
        // e.g.  * ...|
        beforeText: /^(\ \ )*\ \*(\ ([^\*]|\*(?!\/))*)?$/,
        action: { indentAction: vscode.IndentAction.None, appendText: '* ' },
      },
      {
        // Dedents after closing a multi-line comment
        // e.g.  */|
        beforeText: /^(\ \ )*\ \*\/\s*$/,
        action: { indentAction: vscode.IndentAction.None, removeText: 1 },
      },
    ],
  });
}

function activate(context) {
    // The server is implemented in reason

    let clientOptions = {
        documentSelector: [
            {scheme: 'file', language: 'reason'},
            {scheme: 'file', language: 'ocaml'},
            {scheme: 'file', language: 'reason_lisp'}
        ],
        synchronize: {
            // Synchronize the setting section 'reason_language_server' to the server
            configurationSection: 'reason_language_server',
            fileEvents: [
                vscode.workspace.createFileSystemWatcher('**/bsconfig.json'),
                vscode.workspace.createFileSystemWatcher('**/.merlin')
            ]
        },
        revealOutputChannelOn: RevealOutputChannelOn.Never,
    };

    let client = null
    let lastStartTime = null
    let interval = null

    const startChecking = (location) => {
        vscode.window.showInformationMessage('DEBUG MODE: Will auto-restart the reason language server if it recompiles');
        interval = setInterval(() => {
            try {
                const stat = fs.statSync(location)
                const mtime = stat.mtime.getTime()
                if (mtime > lastStartTime) {
                    restart()
                }
            } catch (e) {
                console.warn('Failed to check binary mtime ' + e.message)
            }
        }, 500);
        context.subscriptions.push({dispose: () => clearInterval(checkForBinaryChange)})
    }

    /**
     * client.sendRequest(...).then(response => {
     * })
     *
     * client.sendNotification()....
     * that could be a way to implement the code actions that I want to do.
     *
     * alsooo I think I want
     */

    const restart = () => {
        if (client) {
            client.stop();
        }
        const binLocation = getLocation(context)
        if (!binLocation) return

        vscode.window.showErrorMessage('Starting server: ' + binLocation);
        client = new LanguageClient(
            'reason-language-server',
            'Reason Language Server',
            {
                command: binLocation,
                args: [],
            },
            Object.assign({}, clientOptions, {
                revealOutputChannelOn: vscode.workspace.getConfiguration('reason_language_server').get('show_debug_errors')
                    ? RevealOutputChannelOn.Error
                    : RevealOutputChannelOn.Never
            }),
        );
        lastStartTime = Date.now()
        context.subscriptions.push(client.start());
        if (shouldReload()) {
            vscode.window.showInformationMessage('Reason language server restarted');
            if (!interval) {
                startChecking(location)
            }
        } else if (interval) {
            vscode.window.showInformationMessage('DEBUG MODE OFF - no longer monitoring reason-language-server binary');
            clearInterval(interval)
            interval = null
        }
    }

    vscode.workspace.onDidChangeConfiguration(evt => {
        if (evt.affectsConfiguration('reason_language_server.location')) {
            restart()
        }
    });

    vscode.workspace.onDidChangeWorkspaceFolders(_evt => {
        restart(); 
    });

    vscode.commands.registerCommand('reason-language-server.restart', restart);

    const createInterface = (minimal) => {
        if (!client) {
            return vscode.window.showInformationMessage('Language server not running');
        }
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            return vscode.window.showInformationMessage('No active editor');
        }
        if (fs.existsSync(editor.document.uri.fsPath + 'i')) {
            return vscode.window.showInformationMessage('Interface file already exists');
        }
        client.sendRequest("custom:reasonLanguageServer/createInterface", {
            "uri": editor.document.uri.toString(),
            "minimal": minimal
        })
    };

    vscode.commands.registerCommand('reason-language-server.create_interface', () => {
        createInterface(false)
    });


    class AstSourceProvider {
        constructor() {
            this.privateOnDidChange = new vscode.EventEmitter()
            this.onDidChange = this.privateOnDidChange.event;
        }
        provideTextDocumentContent(uri, token) {
            if (!client) {
                return Promise.reject("No language client running")
            }

            return client.sendRequest("custom:reasonLanguageServer/showAst", {
                "textDocument": {
                    "uri": uri.with({scheme: 'file'}).toString(),
                },
                // unused currently
                "position": {character: 0, line: 0},
            })
        }
    }

    class PpxedSourceProvider {
        constructor() {
            this.privateOnDidChange = new vscode.EventEmitter()
            this.onDidChange = this.privateOnDidChange.event;
        }
        provideTextDocumentContent(uri, token) {
            if (!client) {
                return Promise.reject("No language client running")
            }

            return client.sendRequest("custom:reasonLanguageServer/showPpxedSource", {
                "textDocument": {
                    "uri": uri.with({scheme: 'file'}).toString(),
                },
                // unused currently
                "position": {character: 0, line: 0},
            })
        }
    }

    const provider = new PpxedSourceProvider()
    const astProvider = new AstSourceProvider()

    context.subscriptions.push(
        vscode.workspace.onDidSaveTextDocument((document) => {
            const uri = document.uri;
            provider.privateOnDidChange.fire(uri.with({scheme: 'ppxed-source'}))
            astProvider.privateOnDidChange.fire(uri.with({scheme: 'ast-source'}))
        }),
    );

    context.subscriptions.push(configureLanguage());

    vscode.workspace.registerTextDocumentContentProvider("ppxed-source", provider);
    vscode.workspace.registerTextDocumentContentProvider("ast-source", astProvider);

    const showPpxedSource = () => {
        if (!client) {
            return vscode.window.showInformationMessage('Language server not running');
        }
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            return vscode.window.showInformationMessage('No active editor');
        }
        if (editor.document.languageId !== 'ocaml' && editor.document.languageId !== 'reason') {
            return vscode.window.showInformationMessage('Not an OCaml or Reason file');
        }

        const document = TextDocument.create(editor.document.uri.with({scheme: 'ppxed-source'}), editor.document.languageId, 1, '');
        vscode.window.showTextDocument(document);
    };

    vscode.commands.registerCommand('reason-language-server.show_ppxed_source', showPpxedSource);

    const showAst = () => {
        if (!client) {
            return vscode.window.showInformationMessage('Language server not running');
        }
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            return vscode.window.showInformationMessage('No active editor');
        }
        if (editor.document.languageId !== 'ocaml' && editor.document.languageId !== 'reason') {
            return vscode.window.showInformationMessage('Not an OCaml or Reason file');
        }

        const document = TextDocument.create(editor.document.uri.with({scheme: 'ast-source'}), editor.document.languageId, 1, '');
        vscode.window.showTextDocument(document);
    };

    vscode.commands.registerCommand('reason-language-server.show_ast', showAst);

    // vscode.commands.registerCommand('reason-language-server.create_interface_minimal', () => {
    //     createInterface(true)
    // });

    restart();

    // vscode.commands.registerCommand('reason-language-server.expand_switch', () => {
    // });

}
exports.activate = activate;
