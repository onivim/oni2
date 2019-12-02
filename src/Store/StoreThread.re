/*
 * StoreThread.re
 *
 * This is the 'state management' piece of Oni2.
 *
 * The state updates are run in a parallel thread to the rendering,
 * so that we can eek out as much perf as we can in this architecture.
 */

open Revery;

module Core = Oni_Core;
module Extensions = Oni_Extensions;
module Model = Oni_Model;

open Oni_Extensions;

let discoverExtensions = (setup: Core.Setup.t) => {
  let extensions =
    Core.Log.perf("Discover extensions", () => {
      let extensions = ExtensionScanner.scan(setup.bundledExtensionsPath);
      let userExtensions = Core.Filesystem.getExtensionsFolder();
      let developmentExtensions =
        switch (setup.developmentExtensionsPath) {
        | Some(p) =>
          let ret = ExtensionScanner.scan(p);
          ret;
        | None => []
        };
      let userExtensions =
        switch (userExtensions) {
        | Ok(p) =>
          Core.Log.info("Searching for user extensions in: " ++ p);
          ExtensionScanner.scan(p);
        | Error(msg) =>
          Core.Log.error("Error discovering user extensions: " ++ msg);
          [];
        };

      Core.Log.debug(() =>
        "discoverExtensions - discovered "
        ++ string_of_int(List.length(userExtensions))
        ++ " user extensions."
      );
      [extensions, developmentExtensions, userExtensions] |> List.flatten;
    });

  Core.Log.info(
    "-- Discovered: "
    ++ string_of_int(List.length(extensions))
    ++ " extensions",
  );

  extensions;
};

let start =
    (
      ~configurationFilePath=None,
      ~setup: Core.Setup.t,
      ~executingDirectory,
      ~onStateChanged,
      ~getClipboardText,
      ~setClipboardText,
      ~getZoom,
      ~setZoom,
      ~quit,
      ~getTime,
      ~setTitle,
      ~setVsync,
      ~window: option(Revery.Window.t),
      ~cliOptions: option(Oni_Core.Cli.t),
      ~getScaleFactor,
      (),
    ) => {
  ignore(executingDirectory);

  let state = Model.State.create();

  let (merlinUpdater, merlinStream) = MerlinStoreConnector.start();

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

  let extensions = discoverExtensions(setup);
  let languageInfo = Model.LanguageInfo.ofExtensions(extensions);
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
    SyntaxHighlightingStoreConnector.start(languageInfo, setup);
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

  let ripgrep = Core.Ripgrep.make(~executablePath=setup.rgPath);

  let (fileExplorerUpdater, explorerStream) =
    FileExplorerStore.start();

  let (searchUpdater, searchStream) = SearchStoreConnector.start();

  let (lifecycleUpdater, lifecycleStream) =
    LifecycleStoreConnector.start(quit);
  let indentationUpdater = IndentationStoreConnector.start();
  let (windowUpdater, windowStream) = WindowsStoreConnector.start(getState);

  let fontUpdater = FontStoreConnector.start(~getScaleFactor, ());
  let keyDisplayerUpdater = KeyDisplayerConnector.start(getTime);
  let acpUpdater = AutoClosingPairsConnector.start(languageInfo);

  let completionUpdater = CompletionStoreConnector.start();

  let (hoverUpdater, hoverStream) = HoverStoreConnector.start();

  let inputStream =
    InputStoreConnector.start(getState, window, runRunEffects);

  let titleUpdater = TitleStoreConnector.start(setTitle);

  let (storeDispatch, storeStream) =
    Isolinear.Store.create(
      ~initialState=state,
      ~updater=
        Isolinear.Updater.combine([
          Isolinear.Updater.ofReducer(Reducer.reduce),
          vimUpdater,
          syntaxUpdater,
          extHostUpdater,
          fontUpdater,
          quickmenuUpdater,
          configurationUpdater,
          keyBindingsUpdater,
          commandUpdater,
          lifecycleUpdater,
          fileExplorerUpdater,
          searchUpdater,
          indentationUpdater,
          windowUpdater,
          keyDisplayerUpdater,
          themeUpdater,
          merlinUpdater,
          acpUpdater,
          hoverUpdater,
          completionUpdater,
          titleUpdater,
        ]),
      (),
    );

  module QuickmenuSubscriptionRunner =
    Core.Subscription.Runner({
      type action = Model.Actions.t;
      let id = "quickmenu-subscription";
    });
  let (quickmenuSubscriptionsUpdater, quickmenuSubscriptionsStream) =
    QuickmenuStoreConnector.subscriptions(ripgrep);

  module SearchSubscriptionRunner =
    Core.Subscription.Runner({
      type action = Model.Actions.t;
      let id = "search-subscription";
    });
  let (searchSubscriptionsUpdater, searchSubscriptionsStream) =
    SearchStoreConnector.subscriptions(ripgrep);

  let rec dispatch = (action: Model.Actions.t) => {
    switch (action) {
    | Tick(_) => () // This gets a bit intense, so ignore it
    | _ =>
      Core.Log.info("[StoreThread.dispatch]: " ++ Model.Actions.show(action))
    };

    let lastState = latestState^;
    let (newState, effect) = storeDispatch(action);
    accumulatedEffects := [effect, ...accumulatedEffects^];
    latestState := newState;

    if (newState !== lastState) {
      onStateChanged(newState);
    };

    // TODO: Wire this up properly
    let quickmenuSubs = quickmenuSubscriptionsUpdater(newState);
    QuickmenuSubscriptionRunner.run(~dispatch, quickmenuSubs);
    let searchSubs = searchSubscriptionsUpdater(newState);
    SearchSubscriptionRunner.run(~dispatch, searchSubs);
  };

  let runEffects = () => {
    let effects = accumulatedEffects^;
    accumulatedEffects := [];

    effects
    |> List.filter(e => e != Isolinear.Effect.none)
    |> List.rev
    |> List.iter(e => {
         Core.Log.debug(() =>
           "[StoreThread] Running effect: " ++ Isolinear.Effect.getName(e)
         );
         Isolinear.Effect.run(e, dispatch);
       });
  };

  latestRunEffects := Some(runEffects);

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
    Isolinear.Stream.connect(dispatch, searchStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, lifecycleStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, windowStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, hoverStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, merlinStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, quickmenuSubscriptionsStream);
  let _: Isolinear.Stream.unsubscribeFunc =
    Isolinear.Stream.connect(dispatch, searchSubscriptionsStream);

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

  let totalTime = ref(0.0);
  let _ =
    Tick.interval(
      deltaT => {
        let deltaTime = Time.toFloatSeconds(deltaT);
        totalTime := totalTime^ +. deltaTime;
        dispatch(Model.Actions.Tick({deltaTime, totalTime: totalTime^}));
        runEffects();
      },
      Time.zero,
    );

  (dispatch, runEffects);
};
