/*
 * StoreThread.re
 *
 * This is the 'state management' piece of Oni2.
 *
 * The state updates are run in a parallel thread to the rendering,
 * so that we can eek out as much perf as we can in this architecture.
 */

module Core = Oni_Core;
module Option = Core.Utility.Option;

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
      ~getScaleFactor,
      (),
    ) => {
  ignore(executingDirectory);

  let cliOptions =
    Option.value(
      ~default=Core.Cli.create(~folder="", ~filesToOpen=[], ()),
      cliOptions,
    );

  let state = Model.State.create();

  let accumulatedEffects: ref(list(Isolinear.Effect.t(Model.Actions.t))) =
    ref([]);

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

  let (syntaxUpdater, syntaxStream) =
    SyntaxHighlightingStoreConnector.start(languageInfo, setup, cliOptions);
  let themeUpdater = ThemeStoreConnector.start(themeInfo);

  let (extHostUpdater, extHostStream) =
    ExtensionClientStoreConnector.start(extensions, setup);

  let (quickmenuUpdater, quickmenuStream) =
    QuickmenuStoreConnector.start(themeInfo);

  let configurationUpdater =
    ConfigurationStoreConnector.start(
      ~configurationFilePath,
      ~cliOptions,
      ~getZoom,
      ~setZoom,
      ~setVsync,
    );
  let keyBindingsUpdater = KeyBindingsStoreConnector.start();

  let (fileExplorerUpdater, explorerStream) = FileExplorerStore.start();

  let (lifecycleUpdater, lifecycleStream) =
    LifecycleStoreConnector.start(quit);
  let indentationUpdater = IndentationStoreConnector.start();
  let (windowUpdater, windowStream) = WindowsStoreConnector.start();

  let fontUpdater = FontStoreConnector.start(~getScaleFactor, ());
  let acpUpdater = AutoClosingPairsConnector.start(languageInfo);

  let completionUpdater = CompletionStoreConnector.start();

  let (languageFeatureUpdater, languageFeatureStream) =
    LanguageFeatureConnector.start();

  let (inputUpdater, inputStream) =
    InputStoreConnector.start(window, runRunEffects);

  let titleUpdater = TitleStoreConnector.start(setTitle);
  let sneakUpdater = SneakStore.start();
  let contextMenuUpdater = ContextMenuStore.start();

  let (storeDispatch, storeStream) =
    Isolinear.Store.create(
      ~initialState=state,
      ~updater=
        Isolinear.Updater.combine([
          Isolinear.Updater.ofReducer(Reducer.reduce),
          inputUpdater,
          quickmenuUpdater,
          vimUpdater,
          syntaxUpdater,
          extHostUpdater,
          fontUpdater,
          configurationUpdater,
          keyBindingsUpdater,
          commandUpdater,
          lifecycleUpdater,
          fileExplorerUpdater,
          indentationUpdater,
          windowUpdater,
          themeUpdater,
          acpUpdater,
          languageFeatureUpdater,
          completionUpdater,
          titleUpdater,
          sneakUpdater,
          Features.update,
          contextMenuUpdater,
        ]),
      (),
    );

  let rec dispatch = (action: Model.Actions.t) => {
    DispatchLog.info(Model.Actions.show(action));

    let lastState = latestState^;
    let (newState, effect) = storeDispatch(action);
    accumulatedEffects := [effect, ...accumulatedEffects^];
    latestState := newState;

    if (newState !== lastState) {
      onStateChanged(newState);
    };

    Features.updateSubscriptions(setup, newState, dispatch);

    onAfterDispatch(action);
    runEffects();
  }

  and runEffects = () => {
    let effects = accumulatedEffects^;
    accumulatedEffects := [];

    effects
    |> List.filter(e => e != Isolinear.Effect.none)
    |> List.rev
    |> List.iter(e => {
         Log.debugf(m =>
           m("Running effect: %s", Isolinear.Effect.getName(e))
         );
         Isolinear.Effect.run(e, dispatch);
       });
  };

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

  let editorEventStream =
    Isolinear.Stream.map(storeStream, ((state, action)) =>
      switch (action) {
      | Model.Actions.BufferUpdate(bs) =>
        let buffer = Model.Selectors.getBufferById(state, bs.id);
        Some(Model.Actions.RecalculateEditorView(buffer));
      | Model.Actions.BufferEnter({id, _}, _) =>
        let buffer = Model.Selectors.getBufferById(state, id);
        Some(Model.Actions.RecalculateEditorView(buffer));
      | _ => None
      }
    );

  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, inputStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, vimStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, editorEventStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, syntaxStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, extHostStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, quickmenuStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, explorerStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, lifecycleStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, windowStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, languageFeatureStream);

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

  (dispatch, runEffects);
};
