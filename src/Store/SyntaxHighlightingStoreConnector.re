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

module NativeSyntaxHighlights = Oni_Syntax.NativeSyntaxHighlights;
module Protocol = Oni_Syntax.Protocol;

// TODO:
// - Move updater to Feature_Terminal
// - Change subscription granularity to per-buffer -
// - this could help remove several effects!
let start = (~enabled, languageInfo: Ext.LanguageInfo.t) => {
  let isVersionValid = (updateVersion, bufferVersion) => {
    bufferVersion != (-1) && updateVersion == bufferVersion;
  };

  let getScopeForBuffer = (buffer: Core.Buffer.t) => {
    buffer
    |> Core.Buffer.getFileType
    |> OptionEx.flatMap(fileType =>
         Ext.LanguageInfo.getScopeFromLanguage(languageInfo, fileType)
       );
  };

  let mapUnitEffect:
    Isolinear.Effect.t(unit) => Isolinear.Effect.t(Model.Actions.t) =
    effect => Isolinear.Effect.map(() => Model.Actions.Nothing, effect);

  let syntaxGrammarRepository =
    Oni_Syntax.GrammarRepository.create(languageInfo);

  let getEagerLines = (~scope, ~configuration, ~theme, lines) => {
    let maxLines =
      configuration |> Core.Configuration.getValue(c => c.syntaxEagerMaxLines);
    let maxLineLength =
      configuration
      |> Core.Configuration.getValue(c => c.syntaxEagerMaxLineLength);

    let len = min(Array.length(lines), maxLines);

    let numberOfLinesToHighlight = {
      let rec iter = idx =>
        if (idx >= len) {
          idx;
        } else if (String.length(lines[idx]) > maxLineLength) {
          idx;
        } else {
          iter(idx + 1);
        };

      iter(0);
    };

    if (numberOfLinesToHighlight == 0) {
      [||];
    } else {
      let linesToHighlight =
        Array.sub(lines, 0, numberOfLinesToHighlight - 1);
      let highlights =
        Feature_Syntax.highlight(
          ~scope,
          ~theme,
          ~grammars=syntaxGrammarRepository,
          linesToHighlight,
        );
      highlights;
    };
  };

  let updater = (state: Model.State.t, action) => {
    let default = (state, Isolinear.Effect.none);
    switch (action) {
    // When there is a buffer update, send it over to the syntax highlight
    // strategy to handle the parsing.
    | Model.Actions.BufferUpdate({update, newBuffer, _}) =>
      let lines = Core.Buffer.getLines(newBuffer);
      let version = Core.Buffer.getVersion(newBuffer);
      let scope = getScopeForBuffer(newBuffer);
      if (!isVersionValid(version, update.version)) {
        default;
      } else {
        switch (scope) {
        | None => default
        | Some(scope) =>
          // Eager syntax highlighting
          let syntaxHighlights =
            if (version == 1 && enabled) {
              let highlights =
                getEagerLines(
                  ~scope,
                  ~configuration=state.configuration,
                  ~theme=state.tokenTheme,
                  update.lines,
                );

              let len = Array.length(highlights);

              let newHighlights = ref(state.syntaxHighlights);
              for (i in 0 to len - 1) {
                newHighlights :=
                  Feature_Syntax.setTokensForLine(
                    ~bufferId=update.id,
                    ~line=i,
                    ~tokens=highlights[i],
                    newHighlights^,
                  );
              };
              newHighlights^;
            } else {
              state.syntaxHighlights;
            };

          // TODO: Bring back in different form
          (state, Isolinear.Effect.none);
        /*(
            {...state, syntaxHighlights},
            Service_Syntax.Effect.bufferUpdate(
              state.syntaxClient,
              update,
              lines,
              Some(scope),
            )
            |> mapUnitEffect,
          );*/
        };
      };
    | _ => default
    };
  };

  updater;
};
