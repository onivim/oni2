open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;
open Oni_Model;

module Log = (val Log.withNamespace("Oni2.Extension.ClientStore"));

module CompletionItem = Feature_LanguageSupport.CompletionItem;
module Diagnostic = Feature_LanguageSupport.Diagnostic;
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;

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
    ProviderUtility.runIfSelectorPasses(~buffer, ~selector, () => {
      Exthost.Request.LanguageFeatures.provideCompletionItems(
        ~handle=id,
        ~resource=Buffer.getUri(buffer),
        ~position=Exthost.OneBasedPosition.ofPosition(location),
        ~context=
          Exthost.CompletionContext.{
            triggerKind: Invoke,
            triggerCharacter: None,
          },
        client,
      )
      |> Lwt.map(items => {suggestionsToCompletionItems(items)})
    });
  };
};

module ExtensionDefinitionProvider = {
  let definitionToModel = defs => {
    let def = List.hd(defs);
    let Exthost.DefinitionLink.{uri, range, originSelectionRange, _} = def;
    let Range.{start, _}: EditorCoreTypes.Range.t =
      Exthost.OneBasedRange.toRange(range);

    let originRange: option(EditorCoreTypes.Range.t) =
      originSelectionRange |> Option.map(Exthost.OneBasedRange.toRange);

    LanguageFeatures.DefinitionResult.create(
      ~originRange,
      ~uri,
      ~location=start,
    );
  };

  let create = (id, selector, client, (buffer, location)) => {
    ProviderUtility.runIfSelectorPasses(~buffer, ~selector, () => {
      Exthost.Request.LanguageFeatures.provideDefinition(
        ~handle=id,
        ~resource=Buffer.getUri(buffer),
        ~position=Exthost.OneBasedPosition.ofPosition(location),
        client,
      )
      |> Lwt.map(definitionToModel)
    });
  };
};

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
    ProviderUtility.runIfSelectorPasses(~buffer, ~selector, () => {
      Exthost.Request.LanguageFeatures.provideDocumentHighlights(
        ~handle=id,
        ~resource=Buffer.getUri(buffer),
        ~position=Exthost.OneBasedPosition.ofPosition(location),
        client,
      )
      |> Lwt.map(definitionToModel)
    });
  };
};

module ExtensionFindAllReferencesProvider = {
  let create = (id, selector, client, (buffer, location)) => {
    ProviderUtility.runIfSelectorPasses(~buffer, ~selector, () => {
      Exthost.Request.LanguageFeatures.provideReferences(
        ~handle=id,
        ~resource=Buffer.getUri(buffer),
        ~position=Exthost.OneBasedPosition.ofPosition(location),
        ~context=Exthost.ReferenceContext.{includeDeclaration: true},
        client,
      )
    });
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
    ProviderUtility.runIfSelectorPasses(~buffer, ~selector, () => {
      Exthost.Request.LanguageFeatures.provideDocumentSymbols(
        ~handle=id,
        ~resource=Buffer.getUri(buffer),
        client,
      )
    });
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

  let onRegisterDefinitionProvider = (handle, selector, client) => {
    let id = "exthost." ++ string_of_int(handle);
    let definitionProvider =
      ExtensionDefinitionProvider.create(handle, selector, client);

    dispatch(
      Actions.LanguageFeature(
        LanguageFeatures.DefinitionProviderAvailable(id, definitionProvider),
      ),
    );
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

  let handler: Msg.t => Lwt.t(Reply.t) =
    msg => {
      switch (msg) {
      | FileSystem(msg) => Middleware.filesystem(msg)
      | SCM(msg) =>
        Feature_SCM.handleExtensionMessage(
          ~dispatch=msg => dispatch(Actions.SCM(msg)),
          msg,
        );
        Lwt.return(Reply.okEmpty);

      | LanguageFeatures(
          RegisterDocumentSymbolProvider({handle, selector, label}),
        ) =>
        withClient(
          onRegisterDocumentSymbolProvider(handle, selector, label),
        );
        Lwt.return(Reply.okEmpty);
      | LanguageFeatures(RegisterDefinitionSupport({handle, selector})) =>
        withClient(onRegisterDefinitionProvider(handle, selector));
        Lwt.return(Reply.okEmpty);

      | LanguageFeatures(
          RegisterDocumentHighlightProvider({handle, selector}),
        ) =>
        withClient(onRegisterDocumentHighlightProvider(handle, selector));
        Lwt.return(Reply.okEmpty);
      | LanguageFeatures(RegisterReferenceSupport({handle, selector})) =>
        withClient(onRegisterReferencesProvider(handle, selector));
        Lwt.return(Reply.okEmpty);
      | LanguageFeatures(
          RegisterSuggestSupport({
            handle,
            selector,
            _,
            // TODO: Handle additional configuration from suggest registration!
          }),
        ) =>
        withClient(onRegisterSuggestProvider(handle, selector));
        Lwt.return(Reply.okEmpty);

      | Diagnostics(Clear({owner})) =>
        dispatch(Actions.DiagnosticsClear(owner));
        Lwt.return(Reply.okEmpty);
      | Diagnostics(ChangeMany({owner, entries})) =>
        onDiagnosticsChangeMany(owner, entries);
        Lwt.return(Reply.okEmpty);

      | DocumentContentProvider(
          RegisterTextContentProvider({handle, scheme}),
        ) =>
        dispatch(NewTextContentProvider({handle, scheme}));
        Lwt.return(Reply.okEmpty);

      | DocumentContentProvider(UnregisterTextContentProvider({handle})) =>
        dispatch(LostTextContentProvider({handle: handle}));
        Lwt.return(Reply.okEmpty);

      | Decorations(RegisterDecorationProvider({handle, label})) =>
        dispatch(NewDecorationProvider({handle, label}));
        Lwt.return(Reply.okEmpty);
      | Decorations(UnregisterDecorationProvider({handle})) =>
        dispatch(LostDecorationProvider({handle: handle}));
        Lwt.return(Reply.okEmpty);
      | Decorations(DecorationsDidChange({handle, uris})) =>
        dispatch(DecorationsChanged({handle, uris}));
        Lwt.return(Reply.okEmpty);

      | ExtensionService(
          ExtensionActivationError({extensionId, errorMessage}),
        ) =>
        Log.errorf(m =>
          m(
            "Extension '%s' failed to activate: %s",
            extensionId,
            errorMessage,
          )
        );
        Lwt.return(Reply.okEmpty);
      | ExtensionService(DidActivateExtension({extensionId, _})) =>
        dispatch(
          Actions.Extensions(Feature_Extensions.Activated(extensionId)),
        );
        Lwt.return(Reply.okEmpty);

      | LanguageFeatures(
          RegisterRangeFormattingSupport({handle, selector, displayName, _}),
        ) =>
        dispatch(
          Formatting(
            Feature_Formatting.RangeFormatterAvailable({
              handle,
              selector,
              displayName,
            }),
          ),
        );
        Lwt.return(Reply.okEmpty);
      | LanguageFeatures(
          RegisterDocumentFormattingSupport({
            handle,
            selector,
            displayName,
            _,
          }),
        ) =>
        dispatch(
          Formatting(
            Feature_Formatting.DocumentFormatterAvailable({
              handle,
              selector,
              displayName,
            }),
          ),
        );
        Lwt.return(Reply.okEmpty);
      | LanguageFeatures(RegisterHoverProvider({handle, selector})) =>
        dispatch(
          Actions.Hover(
            Feature_Hover.ProviderRegistered({handle, selector}),
          ),
        );
        Lwt.return(Reply.okEmpty);

      | LanguageFeatures(
          RegisterSignatureHelpProvider({handle, selector, metadata}),
        ) =>
        dispatch(
          Actions.SignatureHelp(
            Feature_SignatureHelp.ProviderRegistered({
              handle,
              selector,
              metadata,
            }),
          ),
        );
        Lwt.return(Reply.okEmpty);

      | MessageService(ShowMessage({severity, message, extensionId, _})) =>
        dispatch(ExtMessageReceived({severity, message, extensionId}));
        Lwt.return(Reply.okEmpty);

      | StatusBar(
          SetEntry({
            id,
            label,
            alignment,
            priority,
            color,
            command,
            tooltip,
            _,
          }),
        ) =>
        let command =
          command |> Option.map(({id, _}: Exthost.Command.t) => id);
        dispatch(
          Actions.StatusBar(
            Feature_StatusBar.ItemAdded(
              Feature_StatusBar.Item.create(
                ~command?,
                ~color?,
                ~tooltip?,
                ~id,
                ~label,
                ~alignment,
                ~priority,
                (),
              ),
            ),
          ),
        );
        Lwt.return(Reply.okEmpty);

      | TerminalService(msg) =>
        Service_Terminal.handleExtensionMessage(msg);
        Lwt.return(Reply.okEmpty);
      | _ => Lwt.return(Reply.okEmpty)
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
      ~version="1.44.5", // TODO: How to keep in sync with bundled version?
      ~parentPid,
      ~logsLocation,
      ~logFile,
      ~logLevel=0,
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

  let env = Luv.Env.environ() |> Result.get_ok;
  let environment = [
    (
      "AMD_ENTRYPOINT",
      "vs/workbench/services/extensions/node/extensionHostProcess",
    ),
    ("VSCODE_IPC_HOOK_EXTHOST", pipeStr),
    ("VSCODE_PARENT_PID", parentPid |> string_of_int),
    ...env,
  ];

  let nodePath = Setup.(setup.nodePath);
  let extHostScriptPath = Setup.getNodeExtensionHostPath(setup);

  let on_exit = (_, ~exit_status: int64, ~term_signal) => {
    Log.infof(m =>
      m(
        "Extension host process exited with exit status: %Ld and signal: %d",
        exit_status,
        term_signal,
      )
    );
  };

  let redirect =
    if (Timber.App.isEnabled()) {
      [
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
      ];
    } else {
      [];
    };

  let _process: Luv.Process.t =
    LuvEx.Process.spawn(
      ~environment,
      ~on_exit,
      ~redirect,
      nodePath,
      [nodePath, extHostScriptPath],
    )
    // TODO: More robust error handling
    |> Result.get_ok;

  client |> Result.iter(c => maybeClientRef := Some(c));

  (client, stream);
};
