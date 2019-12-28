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

module Log = Core.Log;

let start =
    (languageInfo: Ext.LanguageInfo.t, setup: Core.Setup.t, cli: Core.Cli.t) => {
  let (stream, dispatch) = Isolinear.Stream.create();

  if (!cli.shouldSyntaxHighlight) {
    let updater = (state, _action) => (state, Isolinear.Effect.none);
    (updater, stream);
  } else {
    let onHighlights = tokenUpdates => {
      dispatch(Model.Actions.BufferSyntaxHighlights(tokenUpdates));
    };

    let _syntaxClient =
      Oni_Syntax_Client.start(
        ~scheduler=Core.Scheduler.mainThread,
        ~onHighlights,
        ~onHealthCheckResult=_ => (),
        languageInfo,
        setup,
      );

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

    let bufferEnterEffect = (id: int, fileType) =>
      Isolinear.Effect.create(~name="syntax.bufferEnter", () => {
        fileType
        |> Option.iter(fileType =>
             Oni_Syntax_Client.notifyBufferEnter(_syntaxClient, id, fileType)
           )
      });

    let bufferUpdateEffect =
        (bufferUpdate: Oni_Core.BufferUpdate.t, lines, maybeScope) =>
      Isolinear.Effect.create(~name="syntax.bufferUpdate", () => {
        switch (maybeScope) {
        | None => ()
        | Some(scope) =>
          Oni_Syntax_Client.notifyBufferUpdate(
            _syntaxClient,
            bufferUpdate,
            lines,
            scope,
          )
        }
      });

    let configurationChangeEffect = (config: Core.Configuration.t) =>
      Isolinear.Effect.create(~name="syntax.configurationChange", () => {
        Oni_Syntax_Client.notifyConfigurationChanged(_syntaxClient, config)
      });

    let themeChangeEffect = theme =>
      Isolinear.Effect.create(~name="syntax.theme", () => {
        Oni_Syntax_Client.notifyThemeChanged(_syntaxClient, theme)
      });

    let visibilityChangedEffect = visibleRanges =>
      Isolinear.Effect.create(~name="syntax.visibilityChange", () => {
        Oni_Syntax_Client.notifyVisibilityChanged(
          _syntaxClient,
          visibleRanges,
        )
      });

    let isVersionValid = (updateVersion, bufferVersion) => {
      bufferVersion != (-1) && updateVersion == bufferVersion;
    };

    let getScopeForBuffer = (state: Model.State.t, id: int) => {
      state.buffers
      |> Model.Buffers.getBuffer(id)
      |> Option.bind(buf => Core.Buffer.getFileType(buf))
      |> Option.bind(fileType =>
           Ext.LanguageInfo.getScopeFromLanguage(languageInfo, fileType)
         );
    };

    let updater = (state: Model.State.t, action) => {
      let default = (state, Isolinear.Effect.none);
      switch (action) {
      | Model.Actions.ConfigurationSet(config) => (
          state,
          configurationChangeEffect(config),
        )
      | Model.Actions.SetTokenTheme(tokenTheme) => (
          state,
          themeChangeEffect(tokenTheme),
        )
      | Model.Actions.BufferEnter(metadata, fileType) =>
        let visibleBuffers =
          Model.EditorVisibleRanges.getVisibleBuffersAndRanges(state);

        let combinedEffects =
          Isolinear.Effect.batch([
            visibilityChangedEffect(visibleBuffers),
            bufferEnterEffect(Vim.BufferMetadata.(metadata.id), fileType),
          ]);

        (state, combinedEffects);
      // When the view changes, update our list of visible buffers,
      // so we know which ones might have pending work!
      | Model.Actions.EditorGroupAdd(_)
      | Model.Actions.EditorScroll(_)
      | Model.Actions.EditorScrollToLine(_)
      | Model.Actions.EditorScrollToColumn(_)
      | Model.Actions.AddSplit(_)
      | Model.Actions.RemoveSplit(_)
      | Model.Actions.ViewSetActiveEditor(_)
      //| Model.Actions.BufferEnter(_)
      | Model.Actions.ViewCloseEditor(_) =>
        let visibleBuffers =
          Model.EditorVisibleRanges.getVisibleBuffersAndRanges(state);
        (state, visibilityChangedEffect(visibleBuffers));
      // When there is a buffer update, send it over to the syntax highlight
      // strategy to handle the parsing.
      | Model.Actions.BufferUpdate(bu) =>
        let lines = getLines(state, bu.id);
        let version = getVersion(state, bu.id);
        let scope = getScopeForBuffer(state, bu.id);
        if (!isVersionValid(version, bu.version)) {
          default;
        } else {
          (state, bufferUpdateEffect(bu, lines, scope));
        };
      | _ => default
      };
    };

    (updater, stream);
  };
};
