/*
 * StoreThread.re
 *
 * This is the 'state management' piece of Oni2.
 *
 * The state updates are run in a parallel thread to the rendering,
 * so that we can eek out as much perf as we can in this architecture.
 */

module Core = Oni_Core;

module Extensions = Oni_Extensions;
module Model = Oni_Model;

open Oni_Extensions;

module Log = (val Core.Log.withNamespace("Oni2.Store.StoreThread"));
module DispatchLog = (val Core.Log.withNamespace("Oni2.Store.dispatch"));

let discoverExtensions = (setup: Core.Setup.t, cli: Core.Cli.t) =>
  if (cli.shouldLoadExtensions) {
    let extensions =
      Core.Log.perf("Discover extensions", () => {
        let extensions =
          ExtensionScanner.scan(
            // The extension host assumes bundled extensions start with 'vscode.'
            ~category=Bundled,
            ~prefix=Some("vscode"),
            setup.bundledExtensionsPath,
          );

        let developmentExtensions =
          switch (setup.developmentExtensionsPath) {
          | Some(p) => ExtensionScanner.scan(~category=Development, p)
          | None => []
          };

        let userExtensions = Utility.getUserExtensions(cli);

        Log.infof(m =>
          m("Discovered %n user extensions.", List.length(userExtensions))
        );

        [extensions, developmentExtensions, userExtensions] |> List.flatten;
      });

    Log.infof(m =>
      m("-- Discovered: %n extensions", List.length(extensions))
    );

    extensions;
  } else {
    Log.info("Not loading extensions; disabled via CLI");
    [];
  };

let start =
    (
      ~configurationFilePath=None,
      ~onAfterDispatch=_ => (),
      ~setup: Core.Setup.t,
      ~executingDirectory,
      ~onStateChanged,
      ~getClipboardText,
      ~setClipboardText,
      ~getZoom,
      ~setZoom,
      ~quit,
      ~setTitle,
      ~setVsync,
      ~window: option(Revery.Window.t),
      ~cliOptions: option(Oni_Core.Cli.t),
      (),
    ) => {
  ignore(executingDirectory);

  let cliOptions =
    Option.value(
      ~default=Core.Cli.create(~folder="", ~filesToOpen=[], ()),
      cliOptions,
    );

  let state = Model.State.create();

  let latestState: ref(Model.State.t) = ref(state);
  let latestRunEffects: ref(option(unit => unit)) = ref(None);

  let getState = () => latestState^;

  let runRunEffects = () =>
    switch (latestRunEffects^) {
    | Some(v) => v()
    | None => ()
    };

  let extensions = discoverExtensions(setup, cliOptions);
  let languageInfo = LanguageInfo.ofExtensions(extensions);
  let themeInfo = Model.ThemeInfo.ofExtensions(extensions);
  let contributedCommands = Model.Commands.ofExtensions(extensions);

  let commandUpdater =
    CommandStoreConnector.start(getState, contributedCommands);
  let (vimUpdater, vimStream) =
    VimStoreConnector.start(
      languageInfo,
      getState,
      getClipboardText,
      setClipboardText,
    );

  let syntaxUpdater = SyntaxHighlightingStoreConnector.start(languageInfo);
  let themeUpdater = ThemeStoreConnector.start(themeInfo);

  let (extHostClient, extHostStream) =
    ExtensionClient.create(~extensions, ~setup);

  let extHostUpdater =
    ExtensionClientStoreConnector.start(extensions, extHostClient);

  let quickmenuUpdater = QuickmenuStoreConnector.start(themeInfo);

  let configurationUpdater =
    ConfigurationStoreConnector.start(
      ~configurationFilePath,
      ~cliOptions,
      ~getZoom,
      ~setZoom,
      ~setVsync,
    );
  let keyBindingsUpdater = KeyBindingsStoreConnector.start();

  let fileExplorerUpdater = FileExplorerStore.start();

  let lifecycleUpdater = LifecycleStoreConnector.start(quit);
  let indentationUpdater = IndentationStoreConnector.start();
  let windowUpdater = WindowsStoreConnector.start();

  let completionUpdater = CompletionStoreConnector.start();

  let languageFeatureUpdater = LanguageFeatureConnector.start();

  let (inputUpdater, inputStream) =
    InputStoreConnector.start(window, runRunEffects);

  let titleUpdater = TitleStoreConnector.start(setTitle);
  let sneakUpdater = SneakStore.start();
  let contextMenuUpdater = ContextMenuStore.start();
  let updater =
    Isolinear.Updater.combine([
      Isolinear.Updater.ofReducer(Reducer.reduce),
      inputUpdater,
      quickmenuUpdater,
      vimUpdater,
      syntaxUpdater,
      extHostUpdater,
      configurationUpdater,
      keyBindingsUpdater,
      commandUpdater,
      lifecycleUpdater,
      fileExplorerUpdater,
      indentationUpdater,
      windowUpdater,
      themeUpdater,
      languageFeatureUpdater,
      completionUpdater,
      titleUpdater,
      sneakUpdater,
      Features.update(extHostClient),
      contextMenuUpdater,
    ]);

  let subscriptions = (state: Model.State.t) => {
    let syntaxSubscription =
      Feature_Syntax.subscription(
        ~enabled=cliOptions.shouldSyntaxHighlight,
        ~quitting=state.isQuitting,
        ~languageInfo,
        ~setup,
        state.syntaxHighlights,
      )
      |> Isolinear.Sub.map(msg => Model.Actions.Syntax(msg));

    let workspaceUri =
      state.workspace
      |> Option.map((ws: Model.Workspace.workspace) => ws.workingDirectory)
      |> Option.value(~default=Sys.getcwd())
      |> Oni_Core.Uri.fromPath;

    let terminalSubscription =
      Feature_Terminal.subscription(
        ~workspaceUri,
        extHostClient,
        state.terminals,
      )
      |> Isolinear.Sub.map(msg => Model.Actions.Terminal(msg));

    let fontFamily =
      Oni_Core.Configuration.getValue(
        c => c.editorFontFamily,
        state.configuration,
      );
    let fontSize =
      Oni_Core.Configuration.getValue(
        c => c.editorFontSize,
        state.configuration,
      );
    let fontSmoothing =
      Oni_Core.Configuration.getValue(
        c => c.editorFontSmoothing,
        state.configuration,
      );
    let editorFontSubscription =
      Service_Font.Sub.font(~fontFamily, ~fontSize, ~fontSmoothing)
      |> Isolinear.Sub.map(msg => Model.Actions.EditorFont(msg));

    [syntaxSubscription, terminalSubscription, editorFontSubscription]
    |> Isolinear.Sub.batch;
  };

  module Store =
    Isolinear.Store.Make({
      type msg = Model.Actions.t;
      type model = Model.State.t;

      let initial = state;
      let updater = updater;
      let subscriptions = subscriptions;
    });

  let storeStream = Store.Deprecated.getStoreStream();

  let _unsubscribe: unit => unit =
    Store.onModelChanged(newState => {
      latestState := newState;
      onStateChanged(newState);
    });

  let _unsubscribe: unit => unit =
    Store.onBeforeMsg(msg => {DispatchLog.info(Model.Actions.show(msg))});

  let dispatch = Store.dispatch;

  let _unsubscribe: unit => unit =
    Store.onAfterMsg((msg, model) => {
      Features.updateSubscriptions(setup, model, dispatch);
      onAfterDispatch(msg);
      DispatchLog.debugf(m => m("After: %s", Model.Actions.show(msg)));
    });

  let _unsubscribe: unit => unit =
    Store.onBeforeEffectRan(e => {
      Log.debugf(m => m("Running effect: %s", Isolinear.Effect.getName(e)))
    });
  let _unsubscribe: unit => unit =
    Store.onAfterEffectRan(e => {
      Log.debugf(m => m("Effect complete: %s", Isolinear.Effect.getName(e)))
    });

  let runEffects = Store.runPendingEffects;
  latestRunEffects := Some(runEffects);

  Option.iter(
    window =>
      Revery.Window.setCanQuitCallback(window, () =>
        if (Model.Buffers.anyModified(latestState^.buffers)) {
          dispatch(Model.Actions.WindowCloseBlocked);
          false;
        } else {
          true;
        }
      ),
    window,
  );

  // TODO: Remove this wart. There is a complicated timing dependency that shouldn't be necessary.
  let editorEventStream =
    Isolinear.Stream.map(storeStream, ((state, action)) =>
      switch (action) {
      | Model.Actions.BufferUpdate(bs) =>
        let buffer = Model.Selectors.getBufferById(state, bs.update.id);
        Some(Model.Actions.RecalculateEditorView(buffer));
      | Model.Actions.BufferEnter({id, _}, _) =>
        let buffer = Model.Selectors.getBufferById(state, id);
        Some(Model.Actions.RecalculateEditorView(buffer));
      | _ => None
      }
    );

  // TODO: These should all be replaced with isolinear subscriptions.
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, inputStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, vimStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, editorEventStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, extHostStream);

  dispatch(Model.Actions.SetLanguageInfo(languageInfo));

  /* Set icon theme */

  let setIconTheme = s => {
    let iconThemeInfo =
      extensions
      |> List.map((ext: ExtensionScanner.t) =>
           ext.manifest.contributes.iconThemes
         )
      |> List.flatten
      |> List.filter((iconTheme: ExtensionContributions.IconTheme.t) =>
           String.equal(iconTheme.id, s)
         );

    let iconThemeInfo = List.nth_opt(iconThemeInfo, 0);

    switch (iconThemeInfo) {
    | Some(iconThemeInfo) =>
      let iconTheme =
        Yojson.Safe.from_file(iconThemeInfo.path) |> Model.IconTheme.ofJson;

      switch (iconTheme) {
      | Some(iconTheme) => dispatch(Model.Actions.SetIconTheme(iconTheme))
      | None => ()
      };
    | None => ()
    };
  };

  setIconTheme("vs-seti");

  let _: unit => unit =
    Revery.Tick.interval(_ => runEffects(), Revery.Time.zero);

  (dispatch, runEffects);
};
