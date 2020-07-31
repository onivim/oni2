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
      | DownloadService(msg) => Middleware.download(msg)
      | FileSystem(msg) => Middleware.filesystem(msg)
      | SCM(msg) =>
        Feature_SCM.handleExtensionMessage(
          ~dispatch=msg => dispatch(Actions.SCM(msg)),
          msg,
        );
        Lwt.return(Reply.okEmpty);

      | Storage(msg) =>
        let (promise, resolver) = Lwt.task();

        let storageMsg = Feature_Extensions.Msg.storage(~resolver, msg);
        dispatch(Extensions(storageMsg));

        promise;

      | Configuration(RemoveConfigurationOption({key, _})) =>
        dispatch(
          Actions.ConfigurationTransform(
            "configuration.json",
            ConfigurationTransformer.removeField(key),
          ),
        );
        Lwt.return(Reply.okEmpty);

      | Configuration(UpdateConfigurationOption({key, value, _})) =>
        dispatch(
          Actions.ConfigurationTransform(
            "configuration.json",
            ConfigurationTransformer.setField(key, value),
          ),
        );
        Lwt.return(Reply.okEmpty);

      | LanguageFeatures(
          RegisterDocumentSymbolProvider({handle, selector, label}),
        ) =>
        withClient(
          onRegisterDocumentSymbolProvider(handle, selector, label),
        );
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

      | Documents(documentsMsg) =>
        switch (documentsMsg) {
        | Documents.TryOpenDocument({uri}) =>
          if (Oni_Core.Uri.getScheme(uri) == Oni_Core.Uri.Scheme.File) {
            dispatch(
              Actions.OpenFileByPath(
                Oni_Core.Uri.toFileSystemPath(uri),
                None,
                None,
              ),
            );
          } else {
            Log.warnf(m =>
              m(
                "TryOpenDocument: Unable to open %s",
                uri |> Oni_Core.Uri.toString,
              )
            );
          }
        | Documents.TrySaveDocument(_) =>
          Log.warn("TrySaveDocument is not yet implemented.")
        | Documents.TryCreateDocument(_) =>
          Log.warn("TryCreateDocument is not yet implemented.")
        };
        Lwt.return(Reply.okEmpty);

      | ExtensionService(extMsg) =>
        Log.infof(m => m("ExtensionService: %s", Exthost.Msg.show(msg)));
        dispatch(
          Actions.Extensions(Feature_Extensions.Msg.exthost(extMsg)),
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

      | LanguageFeatures(msg) =>
        dispatch(
          Actions.LanguageSupport(Feature_LanguageSupport.Msg.exthost(msg)),
        );
        Lwt.return(Reply.okEmpty);

      | MessageService(msg) =>
        Feature_Messages.Msg.exthost(
          ~dispatch=msg => dispatch(Actions.Messages(msg)),
          msg,
        )
        |> Lwt.map(
             fun
             | None => Reply.okEmpty
             | Some(handle) =>
               Reply.okJson(Exthost.Message.handleToJson(handle)),
           )

      | QuickOpen(msg) =>
        switch (msg) {
        | QuickOpen.Show({instance, _}) =>
          let (promise, resolver) = Lwt.task();
          dispatch(
            QuickmenuShow(
              Extension({id: instance, hasItems: false, resolver}),
            ),
          );

          promise |> Lwt.map(handle => Reply.okJson(`Int(handle)));
        | QuickOpen.SetItems({instance, items}) =>
          dispatch(QuickmenuUpdateExtensionItems({id: instance, items}));
          Lwt.return(Reply.okEmpty);
        | msg =>
          // TODO: Additional quick open messages
          Log.warnf(m =>
            m(
              "Unhandled QuickOpen message: %s",
              Exthost.Msg.QuickOpen.show_msg(msg),
            )
          );
          Lwt.return(Reply.okEmpty);
        }

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
      | Window(OpenUri({uri})) =>
        Service_OS.Api.openURL(uri |> Oni_Core.Uri.toString)
          ? Lwt.return(Reply.okEmpty)
          : Lwt.return(Reply.error("Unable to open URI"))
      | Window(GetWindowVisibility) =>
        Lwt.return(Reply.okJson(`Bool(true)))
      | Workspace(StartFileSearch({includePattern, excludePattern, _})) =>
        Service_OS.Api.glob(
          ~includeFiles=?includePattern,
          ~excludeDirectories=?excludePattern,
          // TODO: Pull from store
          Sys.getcwd(),
        )
        |> Lwt.map(paths =>
             Reply.okJson(
               `List(
                 paths
                 |> List.map(str =>
                      Oni_Core.Uri.fromPath(str) |> Oni_Core.Uri.to_yojson
                    ),
               ),
             )
           )
      | unhandledMsg =>
        Log.warnf(m =>
          m("Unhandled message: %s", Exthost.Msg.show(unhandledMsg))
        );
        Lwt.return(Reply.okEmpty);
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
