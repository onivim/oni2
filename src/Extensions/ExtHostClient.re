/*
 * ExtHostClient.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 *
 */

open Oni_Core;

module Protocol = ExtHostProtocol;
module Workspace = Protocol.Workspace;
module Core = Oni_Core;

module In = Protocol.IncomingNotifications;
module Out = Protocol.OutgoingNotifications;

module Log = (val Log.withNamespace("Oni2.Extensions.ExtHostClient"));

type t = ExtHostTransport.t;

type msg =
  | RegisterSourceControl({
      handle: int,
      id: string,
      label: string,
      rootUri: option(Uri.t),
    })
  | UnregisterSourceControl({handle: int})
  | UpdateSourceControl({
      handle: int,
      hasQuickDiffProvider: option(bool),
      count: option(int),
      commitTemplate: option(string),
    })
  // acceptInputCommand: option(_),
  // statusBarCommands: option(_),
  | RegisterSCMResourceGroup({
      provider: int,
      handle: int,
      id: string,
      label: string,
    })
  | UnregisterSCMResourceGroup({
      provider: int,
      handle: int,
    })
  | SpliceSCMResourceStates({
      provider: int,
      group: int,
      start: int,
      deleteCount: int,
      additions: list(SCMResource.t),
    })
  | RegisterTextContentProvider({
      handle: int,
      scheme: string,
    })
  | UnregisterTextContentProvider({handle: int})
  | RegisterDecorationProvider({
      handle: int,
      label: string,
    })
  | UnregisterDecorationProvider({handle: int})
  | DecorationsDidChange({
      handle: int,
      uris: list(Uri.t),
    });

type unitCallback = unit => unit;
let noop = () => ();
let noop1 = _ => ();
let noop2 = (_, _) => ();

let start =
    (
      ~initialConfiguration,
      ~initData=ExtHostInitData.create(),
      ~initialWorkspace=Workspace.empty,
      ~onInitialized=noop,
      ~onClosed=noop,
      ~onDiagnosticsChangeMany=noop1,
      ~onDiagnosticsClear=noop1,
      ~onDidActivateExtension=noop1,
      ~onExtensionActivationFailed=noop1,
      ~onTelemetry=noop1,
      ~onOutput=noop1,
      ~onRegisterCommand=noop1,
      ~onRegisterDefinitionProvider=noop2,
      ~onRegisterDocumentHighlightProvider=noop2,
      ~onRegisterDocumentSymbolProvider=noop2,
      ~onRegisterReferencesProvider=noop2,
      ~onRegisterSuggestProvider=noop2,
      ~onShowMessage=noop1,
      ~onStatusBarSetEntry,
      ~dispatch,
      setup: Setup.t,
    ) => {
  // Hold onto a reference of the client, so that we can pass it along with
  // '$register' actions.
  let client: ref(option(t)) = ref(None);

  let onMessage = (scope, method, args) => {
    switch (scope, method, args) {
    | ("MainThreadLanguageFeatures", "$registerDocumentSymbolProvider", args) =>
      Option.iter(
        client => {
          In.LanguageFeatures.parseDocumentSymbolProvider(args)
          |> Option.iter(onRegisterDocumentSymbolProvider(client))
        },
        client^,
      );
      Ok(None);

    | ("MainThreadLanguageFeatures", "$registerDefinitionSupport", args) =>
      Option.iter(
        client => {
          In.LanguageFeatures.parseBasicProvider(args)
          |> Option.iter(onRegisterDefinitionProvider(client))
        },
        client^,
      );
      Ok(None);

    | ("MainThreadLanguageFeatures", "$registerReferenceSupport", args) =>
      Option.iter(
        client => {
          In.LanguageFeatures.parseBasicProvider(args)
          |> Option.iter(onRegisterReferencesProvider(client))
        },
        client^,
      );
      Ok(None);

    | (
        "MainThreadLanguageFeatures",
        "$registerDocumentHighlightProvider",
        args,
      ) =>
      Option.iter(
        client => {
          In.LanguageFeatures.parseBasicProvider(args)
          |> Option.iter(onRegisterDocumentHighlightProvider(client))
        },
        client^,
      );
      Ok(None);

    | ("MainThreadLanguageFeatures", "$registerSuggestSupport", args) =>
      Option.iter(
        client => {
          In.LanguageFeatures.parseRegisterSuggestSupport(args)
          |> Option.iter(onRegisterSuggestProvider(client))
        },
        client^,
      );
      Ok(None);

    | ("MainThreadOutputService", "$append", [_, `String(msg)]) =>
      onOutput(msg);
      Ok(None);

    | ("MainThreadDiagnostics", "$changeMany", args) =>
      In.Diagnostics.parseChangeMany(args)
      |> Option.iter(onDiagnosticsChangeMany);
      Ok(None);

    | ("MainThreadDiagnostics", "$clear", args) =>
      In.Diagnostics.parseClear(args) |> Option.iter(onDiagnosticsClear);
      Ok(None);

    | ("MainThreadTelemetry", "$publicLog", [`String(eventName), json]) =>
      onTelemetry(eventName ++ ":" ++ Yojson.Safe.to_string(json));
      Ok(None);

    | (
        "MainThreadMessageService",
        "$showMessage",
        [_level, `String(s), _extInfo, ..._],
      ) =>
      onShowMessage(s);
      Ok(None);

    | ("MainThreadExtensionService", "$onDidActivateExtension", [v, ..._]) =>
      let id = Protocol.PackedString.parse(v);
      onDidActivateExtension(id);
      Ok(None);

    | (
        "MainThreadExtensionService",
        "$onExtensionActivationFailed",
        [v, ..._],
      ) =>
      let id = Protocol.PackedString.parse(v);
      onExtensionActivationFailed(id);
      Ok(None);

    | ("MainThreadCommands", "$registerCommand", [`String(v), ..._]) =>
      onRegisterCommand(v);
      Ok(None);

    | ("MainThreadStatusBar", "$setEntry", args) =>
      In.StatusBar.parseSetEntry(args) |> Option.iter(onStatusBarSetEntry);
      Ok(None);

    | ("MainThreadSCM", "$registerSourceControl", args) =>
      switch (args) {
      | [`Int(handle), `String(id), `String(label), rootUri] =>
        let rootUri = Core.Uri.of_yojson(rootUri) |> Stdlib.Result.to_option;
        dispatch(RegisterSourceControl({handle, id, label, rootUri}));
      | _ =>
        Log.error(
          "Unexpected arguments for MainThreadSCM.$registerSourceControl",
        )
      };
      Ok(None);

    | ("MainThreadSCM", "$unregisterSourceControl", [`Int(handle)]) =>
      dispatch(UnregisterSourceControl({handle: handle}));
      Ok(None);

    | ("MainThreadSCM", "$updateSourceControl", [`Int(handle), features]) =>
      open Yojson.Safe.Util;
      dispatch(
        UpdateSourceControl({
          handle,
          hasQuickDiffProvider:
            features |> member("hasQuickDiffProvider") |> to_bool_option,
          count: features |> member("count") |> to_int_option,
          commitTemplate:
            features |> member("commitTemplate") |> to_string_option,
        }),
      );
      Ok(None);

    | (
        "MainThreadSCM",
        "$registerGroup",
        [`Int(provider), `Int(handle), `String(id), `String(label)],
      ) =>
      dispatch(RegisterSCMResourceGroup({provider, handle, id, label}));
      Ok(None);

    | ("MainThreadSCM", "$unregisterGroup", [`Int(handle), `Int(provider)]) =>
      dispatch(UnregisterSCMResourceGroup({provider, handle}));
      Ok(None);

    | (
        "MainThreadSCM",
        "$spliceResourceStates",
        [`Int(provider), `List(groupSplices)],
      ) =>
      List.iter(
        fun
        | `List([`Int(group), `List(splices)]) =>
          List.iter(
            splice =>
              switch (splice) {
              | `List([`Int(start), `Int(deleteCount), `List(additions)]) =>
                let additions = List.map(In.SCM.parseResource, additions);
                dispatch(
                  SpliceSCMResourceStates({
                    provider,
                    group,
                    start,
                    deleteCount,
                    additions,
                  }),
                );

              | _ => Log.warn("spliceResourceStates: Unexpected json")
              },
            splices,
          )
        | _ => Log.warn("spliceResourceStates: Unexpected json"),
        groupSplices,
      );
      Ok(None);

    | (
        "MainThreadDocumentContentProviders",
        "$registerTextContentProvider",
        [`Int(handle), `String(scheme)],
      ) =>
      dispatch(RegisterTextContentProvider({handle, scheme}));
      Ok(None);

    | (
        "MainThreadDocumentContentProviders",
        "$unregisterTextContentProvider",
        [`Int(handle)],
      ) =>
      dispatch(UnregisterTextContentProvider({handle: handle}));
      Ok(None);

    | (
        "MainThreadDecorations",
        "$registerDecorationProvider",
        [`Int(handle), `String(label)],
      ) =>
      dispatch(RegisterDecorationProvider({handle, label}));
      Ok(None);

    | (
        "MainThreadDecorations",
        "$unregisterDecorationProvider",
        [`Int(handle)],
      ) =>
      dispatch(UnregisterDecorationProvider({handle: handle}));
      Ok(None);

    | (
        "MainThreadDecorations",
        "$onDidChange",
        [`Int(handle), `List(resources)],
      ) =>
      let uris =
        resources
        |> List.filter_map(json =>
             Uri.of_yojson(json) |> Stdlib.Result.to_option
           );
      dispatch(DecorationsDidChange({handle, uris}));
      Ok(None);

    | (scope, method, argsAsJson) =>
      Log.warnf(m =>
        m(
          "Unhandled message - [%s:%s]: %s",
          scope,
          method,
          Yojson.Safe.to_string(`List(argsAsJson)),
        )
      );
      Ok(None);
    };
  };

  let transport =
    ExtHostTransport.start(
      ~initialConfiguration,
      ~initData,
      ~initialWorkspace,
      ~onInitialized,
      ~onMessage,
      ~onClosed,
      setup,
    );
  client := Some(transport);
  transport;
};

let activateByEvent = (evt, client) => {
  ExtHostTransport.send(client, Out.ExtensionService.activateByEvent(evt));
};

let executeContributedCommand = (cmd, client) => {
  ExtHostTransport.send(client, Out.Commands.executeContributedCommand(cmd));
};

let acceptWorkspaceData = (workspace: Workspace.t, client) => {
  ExtHostTransport.send(
    client,
    Out.Workspace.acceptWorkspaceData(workspace),
  );
};

let addDocument = (doc, client) => {
  ExtHostTransport.send(
    client,
    Out.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
      ~addedDocuments=[doc],
      ~removedDocuments=[],
      (),
    ),
  );
};

let updateDocument = (uri, modelChange, dirty, client) => {
  ExtHostTransport.send(
    client,
    Out.Documents.acceptModelChanged(uri, modelChange, dirty),
  );
};

let provideCompletions = (id, uri, position, client) => {
  let f = (json: Yojson.Safe.t) => {
    In.LanguageFeatures.parseProvideCompletionsResponse(json);
  };

  let promise =
    ExtHostTransport.request(
      ~msgType=MessageType.requestJsonArgsWithCancellation,
      client,
      Out.LanguageFeatures.provideCompletionItems(id, uri, position),
      f,
    );
  promise;
};

let provideDecorations = (handle, uri, client) => {
  let decodeItem =
    fun
    | (
        _requestId,
        `List([
          `Int(_),
          `Bool(_),
          `String(tooltip),
          `String(letter),
          `Assoc([("id", `String(color))]),
          `String(source),
        ]),
      ) =>
      Some(Decoration.{handle, tooltip, letter, color, source})
    | (_, json) => {
        Log.error("Unexpected data: " ++ Yojson.Safe.to_string(json));
        None;
      };

  ExtHostTransport.request(
    ~msgType=MessageType.requestJsonArgsWithCancellation,
    client,
    Out.Decorations.provideDecorations(handle, uri),
    json =>
    switch (json) {
    | `Assoc(items) => items |> List.filter_map(decodeItem)

    | _ =>
      failwith(
        Printf.sprintf(
          "Unexpected response from provideDecorations for %s: \n  %s",
          Uri.toString(uri),
          Yojson.Safe.to_string(json),
        ),
      )
    }
  );
};

let provideDefinition = (id, uri, position, client) => {
  let f = (json: Yojson.Safe.t) => {
    let json =
      switch (json) {
      | `List([fst, ..._]) => fst
      | v => v
      };
    Protocol.DefinitionLink.of_yojson_exn(json);
  };

  let promise =
    ExtHostTransport.request(
      ~msgType=MessageType.requestJsonArgsWithCancellation,
      client,
      Out.LanguageFeatures.provideDefinition(id, uri, position),
      f,
    );
  promise;
};

let provideDocumentHighlights = (id, uri, position, client) => {
  let f = (json: Yojson.Safe.t) => {
    switch (json) {
    | `List(items) =>
      List.map(Protocol.DocumentHighlight.of_yojson_exn, items)
    | _ => []
    };
  };

  let promise =
    ExtHostTransport.request(
      ~msgType=MessageType.requestJsonArgsWithCancellation,
      client,
      Out.LanguageFeatures.provideDocumentHighlights(id, uri, position),
      f,
    );
  promise;
};

let provideDocumentSymbols = (id, uri, client) => {
  let f = (json: Yojson.Safe.t) => {
    let default: list(DocumentSymbol.t) = [];
    switch (json) {
    | `List(symbols) => List.map(DocumentSymbol.of_yojson_exn, symbols)
    | _ => default
    };
  };

  let promise =
    ExtHostTransport.request(
      ~msgType=MessageType.requestJsonArgsWithCancellation,
      client,
      Out.LanguageFeatures.provideDocumentSymbols(id, uri),
      f,
    );
  promise;
};

let provideOriginalResource = (id, uri, client) => {
  let promise =
    ExtHostTransport.request(
      ~msgType=MessageType.requestJsonArgsWithCancellation,
      client,
      Out.SCM.provideOriginalResource(id, uri),
      json =>
      Core.Uri.of_yojson(json) |> Stdlib.Result.get_ok
    );
  promise;
};

let provideReferences = (id, uri, position, client) => {
  let f = (json: Yojson.Safe.t) => {
    let default: list(LocationWithUri.t) = [];
    switch (json) {
    | `List(references) =>
      List.map(LocationWithUri.of_yojson_exn, references)
    | _ => default
    };
  };

  let promise =
    ExtHostTransport.request(
      ~msgType=MessageType.requestJsonArgsWithCancellation,
      client,
      Out.LanguageFeatures.provideReferences(id, uri, position),
      f,
    );
  promise;
};

let provideTextDocumentContent = (id, uri, client) => {
  let promise =
    ExtHostTransport.request(
      ~msgType=MessageType.requestJsonArgsWithCancellation,
      client,
      Out.DocumentContent.provideTextDocumentContent(id, uri),
      fun
      | `String(content) => content
      | json =>
        failwith("Unexpected response: " ++ Yojson.Safe.to_string(json)),
    );
  promise;
};

let send = (client, msg) => ExtHostTransport.send(client, msg);

let close = client => ExtHostTransport.close(client);
