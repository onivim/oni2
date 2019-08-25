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
  let extensions = ExtensionScanner.scan(setup.bundledExtensionsPath);
  let developmentExtensions =
    switch (setup.developmentExtensionsPath) {
    | Some(p) =>
      let ret = ExtensionScanner.scan(p);
      ret;
    | None => []
    };

  let extensions = [extensions, developmentExtensions] |> List.flatten;
  Core.Log.debug(
    "-- Discovered: "
    ++ string_of_int(List.length(extensions))
    ++ " extensions",
  );

  extensions;
};

let start =
    (
      ~setup: Core.Setup.t,
      ~executingDirectory,
      ~onStateChanged,
      ~getClipboardText,
      ~setClipboardText,
      ~getTime,
      ~cliOptions: option(Oni_Core.Cli.t),
      ~getScaleFactor,
      (),
    ) => {
  ignore(executingDirectory);

  let state = Model.State.create();

  let accumulatedEffects: ref(list(Isolinear.Effect.t(Model.Actions.t))) =
    ref([]);
  let latestState: ref(Model.State.t) = ref(state);
  let getState = () => latestState^;

  let extensions = discoverExtensions(setup);
  let languageInfo = Model.LanguageInfo.ofExtensions(extensions);

  let commandUpdater = CommandStoreConnector.start(getState);
  let (vimUpdater, vimStream) =
    VimStoreConnector.start(getState, getClipboardText, setClipboardText);

  let (textmateUpdater, textmateStream) =
    TextmateClientStoreConnector.start(languageInfo, setup);

  let themeUpdater = ThemeStoreConnector.start(languageInfo, setup);

  /*
     For our July builds, we won't be including the extension host -
     but we'll bring this back as we start implementing those features!
   */
  /* let (extHostUpdater, extHostStream) =
     ExtensionClientStoreConnector.start(extensions, setup); */

  let (menuHostUpdater, menuStream) = MenuStoreConnector.start();

  let configurationUpdater = ConfigurationStoreConnector.start(~cliOptions);

  let ripgrep = Core.Ripgrep.make(setup.rgPath);
  let quickOpenUpdater = QuickOpenStoreConnector.start(ripgrep);

  let (fileExplorerUpdater, explorerStream) =
    FileExplorerStoreConnector.start();

  let (lifecycleUpdater, lifecycleStream) = LifecycleStoreConnector.start();
  let indentationUpdater = IndentationStoreConnector.start();
  let (windowUpdater, windowStream) = WindowsStoreConnector.start(getState);

  let fontUpdater = FontStoreConnector.start(~getScaleFactor, ());
  let keyDisplayerUpdater = KeyDisplayerConnector.start(getTime);

  let (storeDispatch, storeStream) =
    Isolinear.Store.create(
      ~initialState=state,
      ~updater=
        Isolinear.Updater.combine([
          Isolinear.Updater.ofReducer(Model.Reducer.reduce),
          vimUpdater,
          textmateUpdater,
          /* extHostUpdater, */
          fontUpdater,
          menuHostUpdater,
          quickOpenUpdater,
          configurationUpdater,
          commandUpdater,
          lifecycleUpdater,
          fileExplorerUpdater,
          indentationUpdater,
          windowUpdater,
          keyDisplayerUpdater,
          themeUpdater,
        ]),
      (),
    );

  let editorEventStream =
    Isolinear.Stream.map(storeStream, ((state, action)) =>
      switch (action) {
      | Model.Actions.BufferUpdate(bs) =>
        let buffer = Model.Selectors.getBufferById(state, bs.id);
        Some(Model.Actions.RecalculateEditorView(buffer));
      | Model.Actions.BufferEnter({id, _}) =>
        let buffer = Model.Selectors.getBufferById(state, id);
        Some(Model.Actions.RecalculateEditorView(buffer));
      | _ => None
      }
    );

  let dispatch = (action: Model.Actions.t) => {
    let lastState = latestState^;
    let (newState, effect) = storeDispatch(action);
    accumulatedEffects := [effect, ...accumulatedEffects^];
    latestState := newState;

    if (newState !== lastState) {
      onStateChanged(newState);
    };
  };

  Isolinear.Stream.connect(dispatch, vimStream);
  Isolinear.Stream.connect(dispatch, editorEventStream);
  Isolinear.Stream.connect(dispatch, textmateStream);
  /* Isolinear.Stream.connect(dispatch, extHostStream); */
  Isolinear.Stream.connect(dispatch, menuStream);
  Isolinear.Stream.connect(dispatch, explorerStream);
  Isolinear.Stream.connect(dispatch, lifecycleStream);
  Isolinear.Stream.connect(dispatch, windowStream);

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

  let runEffects = () => {
    let effects = accumulatedEffects^;
    accumulatedEffects := [];

    List.iter(e => Isolinear.Effect.run(e, dispatch), List.rev(effects));
  };

  let _ =
    Tick.interval(
      _ => {
        dispatch(Model.Actions.Tick);
        runEffects();
      },
      Seconds(0.),
    );

  (dispatch, runEffects);
};
