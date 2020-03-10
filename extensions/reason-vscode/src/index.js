/* --------------------------------------------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 * ------------------------------------------------------------------------------------------ */
'use strict';
const vscode = require("vscode");
const {Uri, window} = vscode;
const {LanguageClient, RevealOutputChannelOn} = require("vscode-languageclient");
const {TextDocument} = require('vscode-languageserver-types');
const path = require('path')
const fs = require('fs')
const cp = require('child_process');

const isWindows = process.platform == "win32";

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
    let channel = window.createOutputChannel("reason-vscode");
    context.subscriptions.push(channel);

    let log = (msg) => channel.appendLine(msg);

    const validatePath = (pathToValidate) => {
        if (!pathToValidate) {
            return null;
        }

        if (!path.isAbsolute(pathToValidate)) {
            pathToValidate = path.join(vscode.workspace.rootPath, pathToValidate);
        }

        if (!fs.existsSync(pathToValidate)) {
            return null;
        }

        return pathToValidate;
    };

    const isEsyProject = (projectPath) => {
        const files = fs.readdirSync(projectPath);

        let filtered = files.filter((file) => {
            return file == "dune" || file == "dune-project" || path.extname(file) == ".opam" || file == "esy.lock"
        });
        // We assume if there is an OPAM file or a dune file, this must be a native project.
        const result = filtered.length > 0;
        log(`isEsyProject(${projectPath}): ${result}`);
        return result;
    };

    const isEsyAvailable = (projectPath) => {
        try {
            cp.execSync("esy --version", { cwd: projectPath });
            return true;
        } catch (ex) {
            log("Unable to get esy version: " + ex.toString());
            return false;
        }
    };

    const addExe = (filePath) => 
        isWindows ? filePath + ".exe" : filePath;

    const addCmd = (filePath) => 
        isWindows ? filePath + ".cmd" : filePath;

    const getOcamlLspPath = (projectPath) => {
        try {
           let ocamlLspDirectory = cp.execSync("esy -q sh -c \"echo #{@opam/ocaml-lsp-server.bin}\"", { cwd: projectPath })
           .toString()
           .trim();
            return path.join(ocamlLspDirectory, addExe("ocamllsp"));
        } catch (ex) {
            log("Unable to get ocaml-lsp-server binary: " + ex.toString());
            return null;
        }
    };

    const getLocation = (_context) => {
        let rlsBinaryLocation = validatePath(vscode.workspace.getConfiguration('reason_language_server').get('location'));

        const projectPath = vscode.workspace.rootPath;
        if (isEsyAvailable(projectPath) && isEsyProject(projectPath)) {
            // Let's see if ocaml-lsp-server is available
            log("Found esy. Looking to see if ocaml-lsp-server is available...")
            const ocamlLspPath = getOcamlLspPath(projectPath);
            if (ocamlLspPath) {
                log('Got ocaml LSP binary: ' + ocamlLspPath);
                return [addCmd("esy"), [ocamlLspPath]];
            } else {
                log("ocaml-lsp not found, falling back to RLS: " + rlsBinaryLocation);
                return [rlsBinaryLocation, null]
            }
        } else {
            log("Esy not found, falling back to RLS: " + rlsBinaryLocation);
            return [rlsBinaryLocation, null];
        }
    }

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
        const [command, args] = getLocation(context)
        if (!command) return

        vscode.window.showInformationMessage(`Starting language server: ${command} | ${args}`);
        client = new LanguageClient(
            'reason-language-server',
            'Reason Language Server',
            {
                command,
                args,
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
        log('Workspace root changed:' + vscode.workspace.rootPath);
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

    vscode.commands.registerCommand('reason-language-server.dump_file_data', () => {
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
        client.sendRequest("custom:reasonLanguageServer/dumpFileData", {
            "textDocument": {
                "uri": editor.document.uri.with({scheme: 'file'}).toString(),
            },
        })
    });

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
