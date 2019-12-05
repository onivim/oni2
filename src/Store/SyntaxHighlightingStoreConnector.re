/*
 * SyntaxHighlightingStoreConnector.re
 *
 * This connects syntax highlighting to the store, using various strategies:
 * - ReasonML parser
 * - Tree-Sitter
 * - TextMate grammars
 */

module Core = Oni_Core;
open Core.Utility;

module Model = Oni_Model;
module Ext = Oni_Extensions;

open Oni_Syntax.TreeSitterScopes;
module NativeSyntaxHighlights = Oni_Syntax.NativeSyntaxHighlights;

module Log = Core.Log;

module GrammarRepository = {
  type t = {scopeToGrammar: Core.StringMap.t(Textmate.Grammar.t)};

  let ofLanguageInfo = (languageInfo: Ext.LanguageInfo.t) => {
    let scopeToGrammar: Hashtbl.t(string, Textmate.Grammar.t) =
      Hashtbl.create(32);

    let f = scope => {
      switch (Hashtbl.find_opt(scopeToGrammar, scope)) {
      | Some(v) => Some(v)
      | None =>
        switch (Ext.LanguageInfo.getGrammarPathFromScope(languageInfo, scope)) {
        | Some(grammarPath) =>
          Log.info("GrammarRepository - Loading grammar: " ++ grammarPath);
          let json = Yojson.Safe.from_file(grammarPath);
          let grammar = Textmate.Grammar.Json.of_yojson(json);

          switch (grammar) {
          | Ok(g) =>
            Hashtbl.add(scopeToGrammar, scope, g);
            Some(g);
          | Error(e) =>
            Log.error("Error parsing grammar: " ++ e);
            None;
          };
        | None => None
        }
      };
    };

    f;
  };
};

let start = (languageInfo: Ext.LanguageInfo.t, setup: Core.Setup.t) => {
  let (stream, _dispatch) = Isolinear.Stream.create();
  let _syntaxClient = Oni_Syntax_Client.start(languageInfo);

  //Oni_Syntax_Client.notifyBufferLeave(_syntaxClient, 1);

  let jsonTreeSitterScopes =
    setup.bundledExtensionsPath ++ "/json/syntaxes/tree-sitter-json.json";

  let parsedTreeSitterScopes = Yojson.Safe.from_file(jsonTreeSitterScopes);
  let treeSitterJson =
    Yojson.Safe.Util.member("scopes", parsedTreeSitterScopes);
  let treeSitterScopes = TextMateConverter.of_yojson(treeSitterJson);

  let getTreeSitterScopeMapper = () => {
    treeSitterScopes;
  };

  let getTextmateGrammar = GrammarRepository.ofLanguageInfo(languageInfo);

  let getLines = (state: Model.State.t, id: int) => {
    switch (Model.Buffers.getBuffer(id, state.buffers)) {
    | None => [||]
    | Some(v) => Core.Buffer.getLines(v)
    };
  };

  let getVersion = (state: Model.State.t, id: int) => {
    switch (Model.Buffers.getBuffer(id, state.buffers)) {
    | None => (-1)
    | Some(v) => Core.Buffer.getVersion(v)
    };
  };

  let getScopeForBuffer = (state: Model.State.t, id: int) => {
    switch (Model.Buffers.getBuffer(id, state.buffers)) {
    | None => None
    | Some(buffer) =>
      switch (Core.Buffer.getFileType(buffer)) {
      | None => None
      | Some(v) => Ext.LanguageInfo.getScopeFromLanguage(languageInfo, v)
      }
    };
  };

  let bufferEnterEffect = (state: Model.State.t, id: int, fileType) =>
    Isolinear.Effect.create(~name="syntax.bufferEnter", () => {
      let lines = getLines(state, id);
      fileType
      |> Option.iter(fileType =>
           Oni_Syntax_Client.notifyBufferEnter(
             _syntaxClient,
             id,
             fileType,
             lines,
           )
         );
    });

  let bufferUpdateEffect = (bufferUpdate: Oni_Core.BufferUpdate.t) =>
    Isolinear.Effect.create(~name="syntax.bufferUpdate", () => {
      Oni_Syntax_Client.notifyBufferUpdate(_syntaxClient, bufferUpdate)
    });

  let themeChangeEffect = theme =>
    Isolinear.Effect.create(~name="syntax.theme", () => {
      Oni_Syntax_Client.notifyThemeChanged(_syntaxClient, theme)
    });

  let isVersionValid = (updateVersion, bufferVersion) => {
    bufferVersion != (-1) && updateVersion == bufferVersion;
  };

  let updater = (state: Model.State.t, action) => {
    let default = (state, Isolinear.Effect.none);
    switch (action) {
    | Model.Actions.SetTokenTheme(tokenTheme) => (
        state,
        themeChangeEffect(tokenTheme),
      )
    /*| Model.Actions.Tick(_) =>
      if (Model.SyntaxHighlighting.anyPendingWork(state.syntaxHighlighting)) {
        let syntaxHighlighting =
          Model.SyntaxHighlighting.doPendingWork(state.syntaxHighlighting);
        ({...state, syntaxHighlighting}, Isolinear.Effect.none);
      } else {
        default;
      }*/
    | Model.Actions.BufferEnter(metadata, fileType) => (
        state,
        bufferEnterEffect(state, Vim.BufferMetadata.(metadata.id), fileType),
      )
    // When the view changes, update our list of visible buffers,
    // so we know which ones might have pending work!
    /*| Model.Actions.EditorGroupAdd(_)
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
          syntaxHighlighting:
            Model.SyntaxHighlighting.updateVisibleBuffers(
              visibleBuffers,
              state.syntaxHighlighting,
            ),
        };
        (state, Isolinear.Effect.none);*/
    // When there is a buffer update, send it over to the syntax highlight
    // strategy to handle the parsing.
    | Model.Actions.BufferUpdate(bu) =>
      let lines = getLines(state, bu.id);
      let scope = getScopeForBuffer(state, bu.id);
      let version = getVersion(state, bu.id);
      switch (scope) {
      | None => default
      | Some(scope) when isVersionValid(bu.version, version) =>
        ignore(scope);
        let state = {
          ...state,
          syntaxHighlighting:
            Model.SyntaxHighlighting.onBufferUpdate(
              ~configuration=state.configuration,
              ~scope,
              ~getTextmateGrammar,
              ~getTreeSitterScopeMapper,
              ~bufferUpdate=bu,
              ~lines,
              ~theme=state.tokenTheme,
              state.syntaxHighlighting,
            ),
        };
        (state, bufferUpdateEffect(bu));
      | Some(_) => default
      };
    | _ => default
    };
  };

  (updater, stream);
};
