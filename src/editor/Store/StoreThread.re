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
      ~cliOptions: Core.Cli.t,
      ~setup: Core.Setup.t,
      ~executingDirectory,
      ~onStateChanged,
      (),
    ) => {
  let state = Model.State.create();

  let accumulatedEffects: ref(list(Isolinear.Effect.t(Model.Actions.t))) =
    ref([]);
  let latestState: ref(Model.State.t) = ref(state);

  let extensions = discoverExtensions(setup);

  let (neovimUpdater, neovimStream) =
    NeovimStoreConnector.start(executingDirectory, setup, cliOptions);

  let (textmateUpdater, textmateStream) =
    TextmateClientStoreConnector.start(extensions, setup);

  let (extHostUpdater, extHostStream) =
    ExtensionClientStoreConnector.start(extensions, setup);

  let (menuHostUpdater, menuStream) = MenuStoreConnector.start();

  let ripgrep = Core.Ripgrep.make(setup.rgPath);
  let quickOpenUpdater = QuickOpenStoreConnector.start(ripgrep);

  let (storeDispatch, storeStream) =
    Isolinear.Store.create(
      ~initialState=state,
      ~updater=
        Isolinear.Updater.combine([
          Isolinear.Updater.ofReducer(Model.Reducer.reduce),
          neovimUpdater,
          textmateUpdater,
          extHostUpdater,
          menuHostUpdater,
          quickOpenUpdater,
        ]),
      (),
    );

  let editorEventStream =
    Isolinear.Stream.map(storeStream, ((_state, action)) =>
      switch (action) {
      | SetEditorFont(_)
      | SetEditorSize(_)
      | Model.Actions.BufferUpdate(_)
      | Model.Actions.BufferEnter(_) =>
        Some(Model.Actions.RecalculateEditorView)
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

  Isolinear.Stream.connect(dispatch, neovimStream);
  Isolinear.Stream.connect(dispatch, editorEventStream);
  Isolinear.Stream.connect(dispatch, textmateStream);
  Isolinear.Stream.connect(dispatch, extHostStream);
  Isolinear.Stream.connect(dispatch, menuStream);

  let _ =
    Tick.interval(
      _ => {
        dispatch(Model.Actions.Tick);

        let effects = accumulatedEffects^;
        accumulatedEffects := [];

        List.iter(e => Isolinear.Effect.run(e, dispatch), effects);
      },
      Seconds(0.),
    );

  dispatch;
};
