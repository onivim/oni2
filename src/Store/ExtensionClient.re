open EditorCoreTypes;
open Oni_Core;
open Oni_Model;

module Log = (val Log.withNamespace("Oni2.Extension.ClientStore"));

open Oni_Extensions;
module Extensions = Oni_Extensions;
module Protocol = Extensions.ExtHostProtocol;
module CompletionItem = Feature_LanguageSupport.CompletionItem;
module Diagnostic = Feature_LanguageSupport.Diagnostic;
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;

module Workspace = Protocol.Workspace;

module ExtensionCompletionProvider = {
  let suggestionItemToCompletionItem: Exthost.SuggestItem.t => CompletionItem.t =
    suggestion => {
      {
        label: suggestion.label,
        kind: suggestion.kind,
        detail: suggestion.detail,
      };
    };

  let suggestionsToCompletionItems:
    Exthost.SuggestResult.t => list(CompletionItem.t) =
    ({completions, _}) => {
      completions |> List.map(suggestionItemToCompletionItem);
    };

  let create =
      (
        id: int,
        selector: Exthost.DocumentSelector.t,
        client: Exthost.Client.t,
        (buffer, _completionMeet, location),
      ) => {
    ProviderUtility.runIfSelectorPasses(
      ~buffer,
      ~selector,
      () => {
        let uri = Buffer.getUri(buffer);
        let position = Exthost.OneBasedPosition.ofPosition(location);
        Exthost.Request.LanguageFeatures.provideCompletionItems(
          ~handle=id,
          ~resource=uri,
          ~position,
          ~context=
            Exthost.CompletionContext.{
              triggerKind: Invoke,
              triggerCharacter: None,
            },
          client,
        )
        |> Lwt.map(suggestionsToCompletionItems);
      },
    );
  };
};

// TODO: Properly type definitoin provider...
/*
 module ExtensionDefinitionProvider = {
   let definitionToModel = def => {
     let Protocol.DefinitionLink.{uri, range, originSelectionRange} = def;
     let Range.{start, _} = Protocol.OneBasedRange.toRange(range);

     let originRange =
       originSelectionRange |> Option.map(Protocol.OneBasedRange.toRange);

     LanguageFeatures.DefinitionResult.create(
       ~originRange,
       ~uri,
       ~location=start,
     );
   };

   let create =
       (client, {id, selector}: Protocol.BasicProvider.t, (buffer, location)) => {
     ProviderUtility.runIfSelectorPasses(
       ~buffer,
       ~selector,
       () => {
         let uri = Buffer.getUri(buffer);
         let position = Protocol.OneBasedPosition.ofPosition(location);
         ExtHostClient.provideDefinition(id, uri, position, client)
         |> Lwt.map(definitionToModel);
       },
     );
   };
 };
 */

module ExtensionDocumentHighlightProvider = {
  let definitionToModel = (highlights: list(Exthost.DocumentHighlight.t)) => {
    highlights
    |> List.map(highlight => {
         Exthost.OneBasedRange.toRange(
           Exthost.DocumentHighlight.(highlight.range),
         )
       });
  };

  let create =
      (
        id: int,
        selector: Exthost.DocumentSelector.t,
        client,
        (buffer, location),
      ) => {
    ProviderUtility.runIfSelectorPasses(
      ~buffer,
      ~selector,
      () => {
        let uri = Buffer.getUri(buffer);
        let position = Exthost.OneBasedPosition.ofPosition(location);

        Exthost.Request.LanguageFeatures.provideDocumentHighlights(
          ~handle=id,
          ~resource=uri,
          ~position,
          client,
        )
        |> Lwt.map(definitionToModel);
      },
    );
  };
};

module ExtensionFindAllReferencesProvider = {
  let create = (id, selector, client, (buffer, location)) => {
    ProviderUtility.runIfSelectorPasses(
      ~buffer,
      ~selector,
      () => {
        let uri = Buffer.getUri(buffer);
        let position = Exthost.OneBasedPosition.ofPosition(location);

        Exthost.Request.LanguageFeatures.provideReferences(
          ~handle=id,
          ~resource=uri,
          ~position,
          ~context=Exthost.ReferenceContext.{includeDeclaration: true},
          client,
        );
      },
    );
  };
};

module ExtensionDocumentSymbolProvider = {
  let create =
      (
        id,
        selector,
        _label, // TODO: What to do with label?
        client,
        buffer,
      ) => {
    ProviderUtility.runIfSelectorPasses(
      ~buffer,
      ~selector,
      () => {
        let uri = Buffer.getUri(buffer);
        Exthost.Request.LanguageFeatures.provideDocumentSymbols(
          ~handle=id,
          ~resource=uri,
          client,
        );
      },
    );
  };
};

let create = (~config, ~extensions, ~setup: Setup.t) => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let extensionInfo =
    extensions
    |> List.map(
         ({manifest, path, _}: Exthost.Extension.Scanner.ScanResult.t) =>
         Exthost.Extension.InitData.Extension.ofManifestAndPath(
           manifest,
           path,
         )
       );

  let _onRegisterDefinitionProvider = (client, provider) => {
    ()//      ExtensionDefinitionProvider.create(client, provider);
      //
      //    dispatch(
      //      Actions.LanguageFeature(
      //        LanguageFeatures.DefinitionProviderAvailable(id, definitionProvider),
      //      ),
      // TODO
      //    let id =
      ; //    let definitionProvider =
 //      Protocol.BasicProvider.("exthost." ++ string_of_int(provider.id));
      //    );
  };

  let onRegisterDocumentSymbolProvider = (handle, selector, label, client) => {
    let id = "exthost." ++ string_of_int(handle);
    let documentSymbolProvider =
      ExtensionDocumentSymbolProvider.create(handle, selector, label, client);

    dispatch(
      Actions.LanguageFeature(
        LanguageFeatures.DocumentSymbolProviderAvailable(
          id,
          documentSymbolProvider,
        ),
      ),
    );
  };

  let onRegisterReferencesProvider = (handle, selector, client) => {
    let id = "exthost." ++ string_of_int(handle);
    let findAllReferencesProvider =
      ExtensionFindAllReferencesProvider.create(handle, selector, client);

    dispatch(
      Actions.LanguageFeature(
        LanguageFeatures.FindAllReferencesProviderAvailable(
          id,
          findAllReferencesProvider,
        ),
      ),
    );
  };

  let onRegisterDocumentHighlightProvider = (handle, selector, client) => {
    let id = "exthost." ++ string_of_int(handle);
    let documentHighlightProvider =
      ExtensionDocumentHighlightProvider.create(handle, selector, client);

    dispatch(
      Actions.LanguageFeature(
        LanguageFeatures.DocumentHighlightProviderAvailable(
          id,
          documentHighlightProvider,
        ),
      ),
    );
  };

  let onRegisterSuggestProvider = (handle, selector, client) => {
    let id = "exthost." ++ string_of_int(handle);
    let completionProvider =
      ExtensionCompletionProvider.create(handle, selector, client);

    dispatch(
      Actions.LanguageFeature(
        LanguageFeatures.CompletionProviderAvailable(id, completionProvider),
      ),
    );
  };

  let onDiagnosticsChangeMany =
      (owner: string, entries: list(Exthost.Msg.Diagnostics.entry)) => {
    let protocolDiagToDiag: Exthost.Diagnostic.t => Diagnostic.t =
      d => {
        let range = Exthost.OneBasedRange.toRange(d.range);
        let message = d.message;
        Diagnostic.create(~range, ~message, ());
      };

    let f = (d: Exthost.Msg.Diagnostics.entry) => {
      let diagnostics = List.map(protocolDiagToDiag, snd(d));
      let uri = fst(d);
      Actions.DiagnosticsSet(uri, owner, diagnostics);
    };

    entries |> List.map(f) |> List.iter(a => dispatch(a));
  };
  open Exthost;
  open Exthost.Extension;
  open Exthost.Msg;

  let maybeClientRef = ref(None);

  let withClient = f =>
    switch (maybeClientRef^) {
    | None => Log.warn("Warning - withClient does not have a client")
    | Some(client) => f(client)
    };

  let handler = msg => {
    prerr_endline("GOT MESSAGE: " ++ Exthost.Msg.show(msg));
    switch (msg) {
    //    TODO
    //    | SCM(msg) =>
    //      Feature_SCM.handleExtensionMessage(
    //        ~dispatch=msg => dispatch(Actions.SCM(msg)),
    //        msg,
    //      )

    | LanguageFeatures(
        RegisterDocumentSymbolProvider({handle, selector, label}),
      ) =>
      withClient(onRegisterDocumentSymbolProvider(handle, selector, label));
      None;

    | LanguageFeatures(RegisterDocumentHighlightProvider({handle, selector})) =>
      withClient(onRegisterDocumentHighlightProvider(handle, selector));
      None;
    | LanguageFeatures(RegisterReferenceSupport({handle, selector})) =>
      withClient(onRegisterReferencesProvider(handle, selector));
      None;
    | LanguageFeatures(RegisterSuggestSupport({handle, selector})) =>
      withClient(onRegisterSuggestProvider(handle, selector));
      None;

    | Diagnostics(Clear({owner})) =>
      dispatch(Actions.DiagnosticsClear(owner));
      None;
    | Diagnostics(ChangeMany({owner, entries})) =>
      prerr_endline("Diagnostics - change many!");
      onDiagnosticsChangeMany(owner, entries);
      None;

    | DocumentContentProvider(RegisterTextContentProvider({handle, scheme})) =>
      dispatch(NewTextContentProvider({handle, scheme}));
      None;

    | DocumentContentProvider(UnregisterTextContentProvider({handle})) =>
      dispatch(LostTextContentProvider({handle: handle}));
      None;

    | Decorations(RegisterDecorationProvider({handle, label})) =>
      dispatch(NewDecorationProvider({handle, label}));
      None;
    | Decorations(UnregisterDecorationProvider({handle})) =>
      dispatch(LostDecorationProvider({handle: handle}));
      None;
    | Decorations(DecorationsDidChange({handle, uris})) =>
      dispatch(DecorationsChanged({handle, uris}));
      None;

    | ExtensionService(DidActivateExtension({extensionId, _})) =>
      dispatch(
        Actions.Extension(Oni_Model.Extensions.Activated(extensionId)),
      );
      None;

    | MessageService(ShowMessage({severity, message, extensionId})) =>
      dispatch(ExtMessageReceived({severity, message, extensionId}));
      None;

    | StatusBar(SetEntry({id, text, source, alignment, priority})) =>
      dispatch(
        Actions.StatusBarAddItem(
          StatusBarModel.Item.create(~id, ~text, ~alignment, ~priority, ()),
        ),
      );
      None;

    | TerminalService(msg) =>
      Service_Terminal.handleExtensionMessage(msg);
      None;
    | _ => None
    };
  };

  let parentPid = Luv.Pid.getpid();
  let name = Printf.sprintf("exthost-client-%s", parentPid |> string_of_int);
  let namedPipe = name |> NamedPipe.create;
  let pipeStr = NamedPipe.toString(namedPipe);

  let tempDir = Filename.get_temp_dir_name();

  let logsLocation = tempDir |> Uri.fromPath;
  let logFile =
    Filename.temp_file(~temp_dir=tempDir, "onivim2", "exthost.log")
    |> Uri.fromPath;

  let initData =
    InitData.create(
      ~version="9.9.9", // TODO
      ~parentPid,
      ~logsLocation,
      ~logFile,
      extensionInfo,
    );

  let onError = err => {
    Log.error(err);
  };

  let client =
    Exthost.Client.start(
      ~initialConfiguration=
        Feature_Configuration.toExtensionConfiguration(
          config,
          extensions,
          setup,
        ),
      ~namedPipe,
      ~initData,
      ~handler,
      ~onError,
      (),
    );

  let extHostScriptPath =
    // TODO: Fix this!
    Rench.Path.join(
      Sys.getcwd(),
      "test/collateral/exthost/node_modules/@onivim/vscode-exthost/out/bootstrap-fork.js",
    );

  let environment = [
    ("PATH", Oni_Core.ShellUtility.getPathFromEnvironment()),
    (
      "AMD_ENTRYPOINT",
      "vs/workbench/services/extensions/node/extensionHostProcess",
    ),
    ("VSCODE_IPC_HOOK_EXTHOST", pipeStr),
    ("VSCODE_PARENT_PID", parentPid |> string_of_int),
  ];

  let getNodePath = () => {
    let ic =
      Sys.win32
        // HACK: Not sure why this command doesn't work on Linux / macOS, and vice versa...
        ? Unix.open_process_args_in(
            "node",
            [|"node", "-e", "console.log(process.execPath)"|],
          )
        : Unix.open_process_in("node -e 'console.log(process.execPath)'");
    let nodePath = input_line(ic);
    let _ = close_in(ic);
    nodePath |> String.trim;
  };
  // TODO: Fix provisioning of exthost and use 'real' bundled node
  let nodePath = getNodePath();

  let _process =
    Luv.Process.spawn(
      ~environment,
      ~redirect=[
        Luv.Process.inherit_fd(
          ~fd=Luv.Process.stdin,
          ~from_parent_fd=Luv.Process.stdin,
          (),
        ),
        Luv.Process.inherit_fd(
          ~fd=Luv.Process.stdout,
          ~from_parent_fd=Luv.Process.stderr,
          (),
        ),
        Luv.Process.inherit_fd(
          ~fd=Luv.Process.stderr,
          ~from_parent_fd=Luv.Process.stderr,
          (),
        ),
      ],
      nodePath,
      [nodePath, extHostScriptPath],
      // TODO: More robust error handling
    )
    |> Result.get_ok;

  client |> Result.iter(c => maybeClientRef := Some(c));

  (client, stream);
};
