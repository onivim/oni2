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

module Log = Core.Log;

open Treesitter;

let start = (languageInfo: Model.LanguageInfo.t, setup: Core.Setup.t) => {
  let defaultThemePath =
    setup.bundledExtensionsPath ++ "/onedark-pro/themes/OneDark-Pro.json";

  let (stream, dispatch) = Isolinear.Stream.create();

  let onScopeLoaded = s => Log.debug("Scope loaded: " ++ s);
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
    Isolinear.Effect.create(~name="textmateClient.pump", () =>
      Extensions.TextmateClient.pump(tmClient)
    );

  let notifyBufferUpdateEffect = (scope, buffer, bc) =>
    Isolinear.Effect.create(~name="textmate.notifyBufferUpdate", () => {
      print_endline ("Scope: " ++ scope);

      if (!String.equal(scope, "source.json")) {
        Extensions.TextmateClient.notifyBufferUpdate(tmClient, scope, bc)
      } else {
        let parser = Treesitter.Parser.json();
        /*let startTime = Unix.gettimeofday();
        let _ = Treesitter.ArrayParser.parse(parser, None, Model.Buffer.getLines(buffer));
        let endTime = Unix.gettimeofday();
        print_endline ("time: " ++ string_of_float(endTime -. startTime));*/
        let (tree, _) = Treesitter.ArrayParser.parse(parser, None, Model.Buffer.getLines(buffer));
        let node = Treesitter.Tree.getRootNode(tree);
        print_endline(Treesitter.Node.toString(node));
        ();
      }
    });

  let clearHighlightsEffect = buffer =>
    Isolinear.Effect.create(~name="textmate.clearHighlights", () => {
      let bufferId = Model.Buffer.getId(buffer);
      if (Model.Buffer.isSyntaxHighlightingEnabled(buffer)) {
        Log.debug(
          "Disabling syntax highlighting for buffer: "
          ++ string_of_int(bufferId),
        );
        dispatch(BufferDisableSyntaxHighlighting(bufferId));
        dispatch(SyntaxHighlightClear(bufferId));
      };
    });

  let updater = (state: Model.State.t, action) => {
    let default = (state, Isolinear.Effect.none);
    switch (action) {
    | Model.Actions.Tick => (state, pumpEffect)
    | Model.Actions.BufferUpdate(bc) =>
      let bufferId = bc.id;
      let buffer = Model.Buffers.getBuffer(bufferId, state.buffers);

      switch (buffer) {
      | None => default
      | Some(buffer) =>
        let largeFileOptimizations =
          Model.Selectors.getConfigurationValue(state, buffer, c =>
            c.editorLargeFileOptimizations
          );

        if ((
              !largeFileOptimizations
              || Model.Buffer.getNumberOfLines(buffer)
              < Core.Constants.default.largeFileLineCountThreshold
            )
            && Model.Buffer.isSyntaxHighlightingEnabled(buffer)) {
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
            | Some(scope) => (state, notifyBufferUpdateEffect(scope, buffer,  bc))
            };
          };
        } else {
          (state, clearHighlightsEffect(buffer));
        };
      };

    | _ => default
    };
  };

  (updater, stream);
};
