/*
 * SyntaxHighlightingStoreConnector.re
 *
 * This connects syntax highlighting to the store, using various strategies:
 * - ReasonML parser
 * - Tree-Sitter
 * - TextMate grammars
 */

open Rench;

module Core = Oni_Core;
module Model = Oni_Model;

module Extensions = Oni_Extensions;

open Oni_Syntax.TreeSitterScopes;
module NativeSyntaxHighlights = Oni_Syntax.NativeSyntaxHighlights;

module Log = Core.Log;

let start = (languageInfo: Model.LanguageInfo.t, setup: Core.Setup.t) => {
  let (stream, _dispatch) = Isolinear.Stream.create();
  let jsonTreeSitterScopes =
    setup.bundledExtensionsPath ++ "/json/syntaxes/tree-sitter-json.json";

  let parsedTreeSitterScopes = Yojson.Safe.from_file(jsonTreeSitterScopes);
  let treeSitterJson =
    Yojson.Safe.Util.member("scopes", parsedTreeSitterScopes);
  let treeSitterScopes = TextMateConverter.of_yojson(treeSitterJson);

  let getTreeSitterScopeMapper = () => {
    treeSitterScopes;
  };

  let getLines = (state: Model.State.t, id: int) => {
    switch (Model.Buffers.getBuffer(id, state.buffers)) {
    | None => [||]
    | Some(v) => Model.Buffer.getLines(v)
    };
  };

  let getScopeForBuffer = (state: Model.State.t, id: int) => {
    switch (Model.Buffers.getBuffer(id, state.buffers)) {
    | None => None
    | Some(buffer) =>
      switch (Model.Buffer.getFileType(buffer)) {
      | None => None
      | Some(v) => Model.LanguageInfo.getScopeFromLanguage(languageInfo, v)
      }
    };
  };

  let updater = (state: Model.State.t, action) => {
    let default = (state, Isolinear.Effect.none);
    if (action == Model.Actions.Tick) {
      if (Model.SyntaxHighlighting2.anyPendingWork(state.syntaxHighlighting2)) {
        let syntaxHighlighting2 =
          Model.SyntaxHighlighting2.doPendingWork(state.syntaxHighlighting2);
        print_endline("doing work");
        ({...state, syntaxHighlighting2}, Isolinear.Effect.none);
      } else {
        //print_endline ("NO WORK");
        default;
      };
    } else {
      switch (action) {
      // When the view changes, update our list of visible buffers,
      // so we know which ones might have pending work!
      | Model.Actions.EditorGroupAdd(_)
      | Model.Actions.EditorScroll(_)
      | Model.Actions.EditorScrollToLine(_)
      | Model.Actions.EditorScrollToColumn(_)
      | Model.Actions.AddSplit(_)
      | Model.Actions.RemoveSplit(_)
      | Model.Actions.ViewSetActiveEditor(_)
      | Model.Actions.BufferEnter(_)
      | Model.Actions.ViewCloseEditor(_) =>
        let visibleBuffers =
          Model.EditorVisibleRanges.getVisibleBuffersAndRanges(state);
        let state = {
          ...state,
          syntaxHighlighting2:
            Model.SyntaxHighlighting2.updateVisibleBuffers(
              visibleBuffers,
              state.syntaxHighlighting2,
            ),
        };
        (state, Isolinear.Effect.none);
      // When there is a buffer update, send it over to the syntax highlight
      // strategy to handle the parsing.
      | Model.Actions.BufferUpdate(bu) =>
        let lines = getLines(state, bu.id);
        let scope = getScopeForBuffer(state, bu.id);
        switch (scope) {
        | None => default
        | Some(v) when !NativeSyntaxHighlights.canHandleScope(v) => default
        | Some(v) =>
          let state = {
            ...state,
            syntaxHighlighting2:
              Model.SyntaxHighlighting2.onBufferUpdate(
                ~getTreeSitterScopeMapper,
                ~bufferUpdate=bu,
                ~lines,
                ~theme=state.tokenTheme,
                state.syntaxHighlighting2,
              ),
          };
          (state, Isolinear.Effect.none);
        };
      | _ => default
      };
    };
  };

  (updater, stream);
};
