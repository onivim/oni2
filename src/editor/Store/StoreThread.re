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

let start = (~setup: Core.Setup.t, ~executingDirectory, ~onStateChanged, ()) => {
  let state = Model.State.create();

  let accumulatedEffects: ref(list(Isolinear.Effect.t)) = ref([]);
  let latestState: ref(Model.State.t) = ref(state);

  let extensions = ExtensionScanner.scan(setup.bundledExtensionsPath);
  let developmentExtensions =
    switch (setup.developmentExtensionsPath) {
    | Some(p) =>
      let ret = ExtensionScanner.scan(p);
      ret;
    | None => []
    };

  let extensions = [extensions, developmentExtensions] |> List.flatten;

  let (neovimUpdater, neovimStream) =
    NeovimStoreConnector.start(executingDirectory, setup);

  let (textmateUpdater, textmateStream) =
      TextmateClientStoreConnector.start(extensions, setup);

  let (storeDispatch, storeStream) =
    Isolinear.Store.create(
      ~initialState=state,
      ~updater=
        Isolinear.Updater.combine([
          Isolinear.Updater.ofReducer(Model.Reducer.reduce),
          neovimUpdater,
          textmateUpdater,
        ]),
      (),
    );

  let editorEventStream = Isolinear.Stream.map(storeStream, ((_state, action)) => {
        switch (action) {
        | SetEditorFont(_)
        | SetEditorSize(_)
        | Model.Actions.BufferUpdate(_)
        | Model.Actions.BufferEnter(_) => Some(Model.Actions.RecalculateEditorView)
        | _ => None
        };
  });

  let dispatch = (action: Model.Actions.t) => {
    let (newState, effect) = storeDispatch(action);
    accumulatedEffects := [effect, ...accumulatedEffects^];
    latestState := newState;
    onStateChanged(newState);
  };

  Isolinear.Stream.connect(dispatch, neovimStream);
  Isolinear.Stream.connect(dispatch, editorEventStream);
  Isolinear.Stream.connect(dispatch, textmateStream);


  let languageInfo = Model.LanguageInfo.ofExtensions(extensions);

  Core.Log.debug(
    "-- Discovered: "
    ++ string_of_int(List.length(extensions))
    ++ " extensions",
  );

  let defaultThemePath =
    setup.bundledExtensionsPath ++ "/onedark-pro/themes/OneDark-Pro.json";

  let onScopeLoaded = s => prerr_endline("Scope loaded: " ++ s);
  let onColorMap = cm => dispatch(Model.Actions.SyntaxHighlightColorMap(cm));

  let onTokens = tr => dispatch(Model.Actions.SyntaxHighlightTokens(tr));

  let grammars = Model.LanguageInfo.getGrammars(languageInfo);

  let tmClient =
    Extensions.TextmateClient.start(
      ~onScopeLoaded,
      ~onColorMap,
      ~onTokens,
      setup,
      grammars,
    );

  /* let onExtHostClosed = () => print_endline("ext host closed"); */

  /* let extensionInfo = */
  /*   extensions */
  /*   |> List.map(ext => */
  /*        Extensions.ExtensionHostInitData.ExtensionInfo.ofScannedExtension( */
  /*          ext, */
  /*        ) */
  /*      ); */

  /* let onMessage = (scope, method, args) => { */
  /*   switch (scope, method, args) { */
  /*   | ( */
  /*       "MainThreadStatusBar", */
  /*       "$setEntry", */
  /*       [ */
  /*         `Int(id), */
  /*         _, */
  /*         `String(text), */
  /*         _, */
  /*         _, */
  /*         _, */
  /*         `Int(alignment), */
  /*         `Int(priority), */
  /*       ], */
  /*     ) => */
  /*     dispatch( */
  /*       Model.Actions.StatusBarAddItem( */
  /*         Model.StatusBarModel.Item.create( */
  /*           ~id, */
  /*           ~text, */
  /*           ~alignment=Model.StatusBarModel.Alignment.ofInt(alignment), */
  /*           ~priority, */
  /*           (), */
  /*         ), */
  /*       ), */
  /*     ); */
  /*     Ok(None); */
  /*   | _ => Ok(None) */
  /*   }; */
  /* }; */

  /* let initData = ExtensionHostInitData.create(~extensions=extensionInfo, ()); */
  /* let extHostClient = */
  /*   Extensions.ExtensionHostClient.start( */
  /*     ~initData, */
  /*     ~onClosed=onExtHostClosed, */
  /*     ~onMessage, */
  /*     setup, */
  /*   ); */

  Extensions.TextmateClient.setTheme(tmClient, defaultThemePath);

  let _ =
    Tick.interval(
      _ =>
        {
          dispatch(Model.Actions.Tick);

          let effects = accumulatedEffects^;
          accumulatedEffects := [];

          List.iter(e => Isolinear.Effect.run(e), effects);
          Extensions.TextmateClient.pump(tmClient);
        },
        /* Extensions.ExtensionHostClient.pump(extHostClient); */
      Seconds(0.),
    );


        /* /* TODO:*/
        /*  * Refactor this into a middleware concept, like Redux */*/
        /* switch (msg) {*/
        /* | SetEditorFont(_)*/
        /* | SetEditorSize(_)*/
        /* | Model.Actions.BufferUpdate(_)*/
        /* | Model.Actions.BufferEnter(_) => dispatch(RecalculateEditorView)*/
        /* | _ => ()*/
        /* };*/
        /* /* prerr_endline("Protocol Notification: " ++ Notification.show(n)); */*/

        /* /* TODO:*/
        /*  * Refactor this into _another_ middleware*/
        /*  */*/
        /* switch (msg) {*/
        /* | BufferUpdate(bc) =>*/
        /*   let bufferId = bc.id;*/
        /*   let state = latestState^;*/
        /*   let buffer = Model.BufferMap.getBuffer(bufferId, state.buffers);*/

        /*   switch (buffer) {*/
        /*   | None => ()*/
        /*   | Some(buffer) =>*/
        /*     switch (Model.Buffer.getMetadata(buffer).filePath) {*/
        /*     | None => ()*/
        /*     | Some(v) =>*/
        /*       let extension = Path.extname(v);*/
        /*       switch (*/
        /*         Model.LanguageInfo.getScopeFromExtension(*/
        /*           languageInfo,*/
        /*           extension,*/
        /*         )*/
        /*       ) {*/
        /*       | None => ()*/
        /*       | Some(scope) =>*/
        /*         Extensions.TextmateClient.notifyBufferUpdate(*/
        /*           tmClient,*/
        /*           scope,*/
        /*           bc,*/
        /*         )*/
        /*       };*/
        /*     }*/
        /*   };*/

        /* | _ => ()*/
        /* };*/

  dispatch;
};
