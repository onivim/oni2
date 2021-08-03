type handler('a) = (string, Yojson.Safe.t) => result('a, string);
type mapper('a) = 'a => Msg.t;

type t =
  | MainThreadHandler({
      handler: handler('a),
      mapper: mapper('a),
      id: int,
      name: string,
    })
    : t
  | ExtHostHandler({
      id: int,
      name: string,
    })
    : t;

let getName =
  fun
  | MainThreadHandler({name, _}) => name
  | ExtHostHandler({name, _}) => name;

let getId =
  fun
  | MainThreadHandler({id, _}) => id
  | ExtHostHandler({id, _}) => id;

let setId = (~id) =>
  fun
  | MainThreadHandler(main) => MainThreadHandler({...main, id})
  | ExtHostHandler(ext) => ExtHostHandler({...ext, id});

let defaultHandler = (method, json) => {
  Ok(Msg.Unknown({method, args: json}));
};

let defaultMapper = v => v;

let mainNotImplemented = name => {
  MainThreadHandler({
    id: (-1),
    name,
    handler: defaultHandler,
    mapper: defaultMapper,
  });
};

let main = (~handler, ~mapper, name) => {
  MainThreadHandler({id: (-1), name, handler, mapper});
};

let ext = name => {
  ExtHostHandler({id: (-1), name});
};

/**
 *
 * MUST BE KEPT IN SYNC WITH:
 * https://github.com/microsoft/vscode/blob/8ceb90a807cf96d34bfbe8b048f30e5a7bc50fd2/src/vs/workbench/api/common/extHost.protocol.ts
 *
 * IDs are generated sequentially in both that code and this code - so if they are in sync, the ids will line up.
 * Important for mapping string <-> ids for extension capabilities.
 */
let handlers =
  [
    mainNotImplemented("MainThreadAuthentication"),
    mainNotImplemented("MainThreadBulkEdits"),
    main(
      ~handler=Msg.Clipboard.handle,
      ~mapper=msg => Msg.Clipboard(msg),
      "MainThreadClipboard",
    ),
    main(
      ~handler=Msg.Commands.handle,
      ~mapper=msg => Msg.Commands(msg),
      "MainThreadCommands",
    ),
    mainNotImplemented("MainThreadComments"),
    main(
      ~handler=Msg.Configuration.handle,
      ~mapper=msg => Msg.Configuration(msg),
      "MainThreadConfiguration",
    ),
    main(
      ~handler=Msg.Console.handle,
      ~mapper=msg => Msg.Console(msg),
      "MainThreadConsole",
    ),
    main(
      ~handler=Msg.DebugService.handle,
      ~mapper=msg => Msg.DebugService(msg),
      "MainThreadDebugService",
    ),
    main(
      ~handler=Msg.Decorations.handle,
      ~mapper=msg => Msg.Decorations(msg),
      "MainThreadDecorations",
    ),
    main(
      ~handler=Msg.Diagnostics.handle,
      ~mapper=msg => Msg.Diagnostics(msg),
      "MainThreadDiagnostics",
    ),
    mainNotImplemented("MainThreadDialogs"),
    main(
      ~handler=Msg.Documents.handle,
      ~mapper=msg => Msg.Documents(msg),
      "MainThreadDocuments",
    ),
    main(
      ~handler=Msg.DocumentContentProvider.handle,
      ~mapper=msg => Msg.DocumentContentProvider(msg),
      "MainThreadDocumentContentProviders",
    ),
    main(
      ~handler=Msg.TextEditors.handle,
      ~mapper=msg => Msg.TextEditors(msg),
      "MainThreadTextEditors",
    ),
    mainNotImplemented("MainThreadEditorInsets"),
    mainNotImplemented("MainThreadEditorTabs"),
    main(
      ~handler=Msg.Errors.handle,
      ~mapper=msg => Msg.Errors(msg),
      "MainThreadErrors",
    ),
    mainNotImplemented("MainThreadTreeViews"),
    main(
      ~handler=Msg.DownloadService.handle,
      ~mapper=msg => Msg.DownloadService(msg),
      "MainThreadDownloadService",
    ),
    mainNotImplemented("MainThreadKeytar"),
    main(
      ~handler=Msg.LanguageFeatures.handle,
      ~mapper=msg => Msg.LanguageFeatures(msg),
      "MainThreadLanguageFeatures",
    ),
    main(
      ~handler=Msg.Languages.handle,
      ~mapper=msg => Msg.Languages(msg),
      "MainThreadLanguages",
    ),
    mainNotImplemented("MainThreadLog"),
    main(
      ~handler=Msg.MessageService.handle,
      ~mapper=msg => Msg.MessageService(msg),
      "MainThreadMessageService",
    ),
    main(
      ~handler=Msg.OutputService.handle,
      ~mapper=msg => Msg.OutputService(msg),
      "MainThreadOutputService",
    ),
    main(
      ~handler=Msg.Progress.handle,
      ~mapper=msg => Msg.Progress(msg),
      "MainThreadProgress",
    ),
    main(
      ~handler=Msg.QuickOpen.handle,
      ~mapper=msg => Msg.QuickOpen(msg),
      "MainThreadQuickOpen",
    ),
    main(
      ~handler=Msg.StatusBar.handle,
      ~mapper=msg => Msg.StatusBar(msg),
      "MainThreadStatusBar",
    ),
    mainNotImplemented("MainThreadSecretState"),
    main(
      ~handler=Msg.Storage.handle,
      ~mapper=msg => Msg.Storage(msg),
      "MainThreadStorage",
    ),
    main(
      ~handler=Msg.Telemetry.handle,
      ~mapper=msg => Msg.Telemetry(msg),
      "MainThreadTelemetry",
    ),
    main(
      ~handler=Msg.TerminalService.handle,
      ~mapper=msg => Msg.TerminalService(msg),
      "MainThreadTerminalService",
    ),
    mainNotImplemented("MainThreadWebviews"),
    mainNotImplemented("MainThreadWebviewPanels"),
    mainNotImplemented("MainThreadWebviewViews"),
    mainNotImplemented("MainThreadCustomEditors"),
    mainNotImplemented("MainThreadUrls"),
    mainNotImplemented("MainThreadUriOpeners"),
    main(
      ~handler=Msg.Workspace.handle,
      ~mapper=msg => Msg.Workspace(msg),
      "MainThreadWorkspace",
    ),
    main(
      ~handler=Msg.FileSystem.handle,
      ~mapper=msg => Msg.FileSystem(msg),
      "MainThreadFileSystem",
    ),
    main(
      ~handler=Msg.ExtensionService.handle,
      ~mapper=msg => Msg.ExtensionService(msg),
      "MainThreadExtensionService",
    ),
    main(
      ~handler=Msg.SCM.handle,
      ~mapper=msg => Msg.SCM(msg),
      "MainThreadSCM",
    ),
    mainNotImplemented("MainThreadSearch"),
    mainNotImplemented("MainThreadTask"),
    main(
      ~handler=Msg.Window.handle,
      ~mapper=msg => Msg.Window(msg),
      "MainThreadWindow",
    ),
    mainNotImplemented("MainThreadLabelService"),
    mainNotImplemented("MainThreadNotebook"),
    mainNotImplemented("MainThreadNotebookDocuments"),
    mainNotImplemented("MainThreadNotebookEditors"),
    mainNotImplemented("MainThreadNotebookKernels"),
    mainNotImplemented("MainThreadNotebookRenderers"),
    mainNotImplemented("MainThreadTheming"),
    mainNotImplemented("MainThreadTunnelService"),
    mainNotImplemented("MainThreadTimeline"),
    mainNotImplemented("MainThreadTesting"),
    ext("ExtHostCommands"),
    ext("ExtHostConfiguration"),
    ext("ExtHostDiagnostics"),
    ext("ExtHostDebugService"),
    ext("ExtHostDecorations"),
    ext("ExtHostDocumentsAndEditors"),
    ext("ExtHostDocuments"),
    ext("ExtHostDocumentContentProviders"),
    ext("ExtHostDocumentSaveParticipant"),
    ext("ExtHostEditors"),
    ext("ExtHostTreeViews"),
    ext("ExtHostFileSystem"),
    ext("ExtHostFileSystemInfo"),
    ext("ExtHostFileSystemEventService"),
    ext("ExtHostLanguageFeatures"),
    ext("ExtHostQuickOpen"),
    ext("ExtHostExtensionService"),
    ext("ExtHostLogService"),
    ext("ExtHostTerminalService"),
    ext("ExtHostSCM"),
    ext("ExtHostSearch"),
    ext("ExtHostTask"),
    ext("ExtHostWorkspace"),
    ext("ExtHostWindow"),
    ext("ExtHostWebviews"),
    ext("ExtHostWebviewPanels"),
    ext("ExtHostCustomEditors"),
    ext("ExtHostWebviewViews"),
    ext("ExtHostEditorInsets"),
    ext("ExtHostEditorTabs"),
    ext("ExtHostProgress"),
    ext("ExtHostComments"),
    ext("ExtHostSecretState"),
    ext("ExtHostStorage"),
    ext("ExtHostUrls"),
    ext("ExtHostUriOpeners"),
    ext("ExtHostOutputService"),
    ext("ExtHosLabelService"), // SIC
    ext("ExtHostNotebook"),
    ext("ExtHostNotebookKernels"),
    ext("ExtHostNotebookRenderers"),
    ext("ExtHostTheming"),
    ext("ExtHostTunnelService"),
    ext("ExtHostAuthentication"),
    ext("ExtHostTimeline"),
    ext("ExtHostTesting"),
    ext("ExtHostTelemetry"),
  ]
  |> List.mapi((idx, v) => setId(~id=idx + 1, v));

module Internal = {
  let stringToId =
    handlers
    |> List.map(handler => (getName(handler), getId(handler)))
    |> List.to_seq
    |> Hashtbl.of_seq;

  let idToHandler =
    handlers
    |> List.map(handler => (getId(handler), handler))
    |> List.to_seq
    |> Hashtbl.of_seq;
};

let stringToId = Hashtbl.find_opt(Internal.stringToId);

let handle = (rpcId, method, args) => {
  rpcId
  |> Hashtbl.find_opt(Internal.idToHandler)
  |> Option.to_result(
       ~none="No handler registered for: " ++ (rpcId |> string_of_int),
     )
  |> (
    opt =>
      Result.bind(
        opt,
        fun
        | MainThreadHandler({handler, mapper, _}) => {
            handler(method, args) |> Result.map(mapper);
          }
        | _ => {
            Error("ExtHost handler was incorrectly registered for rpcId");
          },
      )
  );
};
