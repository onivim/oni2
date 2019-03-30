/*
 * TextmateClientStoreConnector.re
 *
 * This connects the textmate client to the store:
 * - Converts textmate notifications into ACTIONS
 * - Calls appropriate APIs on textmate client based on ACTIONS
 */

open Rench;

module Core = Oni_Core;
module Model = Oni_Model;

module Extensions = Oni_Extensions;

let start = (extensions, setup: Core.Setup.t) => {

  let languageInfo = Model.LanguageInfo.ofExtensions(extensions);

  Core.Log.debug(
    "-- Discovered: "
    ++ string_of_int(List.length(extensions))
    ++ " extensions",
  );

  let defaultThemePath =
    setup.bundledExtensionsPath ++ "/onedark-pro/themes/OneDark-Pro.json";

  let _dispatch = ref(None);
  let dispatch = (action) => {
      switch (_dispatch^) {
      | None => ()
      | Some(v) => v(action);
      }
  };

  let stream = Isolinear.Stream.create((dispatch) => {
      _dispatch := Some(dispatch);
  });

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

  Extensions.TextmateClient.setTheme(tmClient, defaultThemePath);

  let pumpEffect =
    Isolinear.Effect.create(~name="textmateClient.pump", () => Extensions.TextmateClient.pump(tmClient));

  let notifyBufferUpdateEffect = (scope, bc) => Isolinear.Effect.create(~name="textmate.notifyBufferUpdate", () => Extensions.TextmateClient.notifyBufferUpdate(tmClient, scope, bc));
          
  let updater = (state: Model.State.t, action) => {
    let default = (state, Isolinear.Effect.none);
    switch (action) {
    | Model.Actions.Tick => (state, pumpEffect)
    | Model.Actions.BufferUpdate(bc) =>
      let bufferId = bc.id;
      let buffer = Model.BufferMap.getBuffer(bufferId, state.buffers);

      switch (buffer) {
      | None => default
      | Some(buffer) =>
        switch (Model.Buffer.getMetadata(buffer).filePath) {
        | None => default
        | Some(v) =>
          let extension = Path.extname(v);
          switch (
            Model.LanguageInfo.getScopeFromExtension(
              languageInfo,
              extension,
            )
          ) {
          | None => default
          | Some(scope) => (state, notifyBufferUpdateEffect(scope, bc));
          };
        }
      };

    | _ => default
    };
  };

  (updater, stream);
};
