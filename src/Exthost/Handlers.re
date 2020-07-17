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
    mainNotImplemented("MainThreadConfiguration"),
    mainNotImplemented("MainThreadConsole"),
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
    mainNotImplemented("MainThreadDocuments"),
    main(
      ~handler=Msg.DocumentContentProvider.handle,
      ~mapper=msg => Msg.DocumentContentProvider(msg),
      "MainThreadDocumentContentProviders",
    ),
    mainNotImplemented("MainThreadTextEditors"),
    mainNotImplemented("MainThreadEditorInsets"),
    mainNotImplemented("MainThreadErrors"),
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
    mainNotImplemented("MainThreadLanguages"),
    mainNotImplemented("MainThreadLog"),
    main(
      ~handler=Msg.MessageService.handle,
      ~mapper=msg => Msg.MessageService(msg),
      "MainThreadMessageService",
    ),
    mainNotImplemented("MainThreadOutputService"),
    mainNotImplemented("MainThreadProgress"),
    mainNotImplemented("MainThreadQuickOpen"),
    main(
      ~handler=Msg.StatusBar.handle,
      ~mapper=msg => Msg.StatusBar(msg),
      "MainThreadStatusBar",
    ),
    mainNotImplemented("MainThreadStorage"),
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
    mainNotImplemented("MainThreadUrls"),
    mainNotImplemented("MainThreadWorkspace"),
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
    mainNotImplemented("MainThreadWindow"),
    mainNotImplemented("MainThreadLabelService"),
    mainNotImplemented("MainThreadNotebook"),
    mainNotImplemented("MainThreadTheming"),
    mainNotImplemented("MainThreadTunnelService"),
    mainNotImplemented("MainThreadTimeline"),
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
    ext("ExtHostEditorInsets"),
    ext("ExtHostProgress"),
    ext("ExtHostComments"),
    ext("ExtHostStorage"),
    ext("ExtHostUrls"),
    ext("ExtHostOutputService"),
    ext("ExtHosLabelService"), // SIC
    ext("ExtHostNotebook"),
    ext("ExtHostTheming"),
    ext("ExtHostTunnelService"),
    ext("ExtHostAuthentication"),
    ext("ExtHostTimeline"),
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
