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

type subscriptionParams = {
  id: string,
  languageInfo: Ext.LanguageInfo.t,
  setup: Core.Setup.t,
  onStart: Oni_Syntax_Client.t => Model.Actions.t,
  onClose: unit => Model.Actions.t,
  onHighlights: list(Oni_Syntax.Protocol.TokenUpdate.t) => Model.Actions.t,
};

module Subscription =
  Isolinear.Sub.Make({
    type msg = Model.Actions.t;

    type params = subscriptionParams;

    type state = Oni_Syntax_Client.t;

    let subscriptionName = "SyntaxSubscription";
    let getUniqueId = params => params.id;

    let init = (~params, ~dispatch) => {
      let client =
        Oni_Syntax_Client.start(
          ~onClose=_ => dispatch(params.onClose()),
          ~scheduler=Core.Scheduler.mainThread,
          ~onHighlights=
            highlights => {dispatch(params.onHighlights(highlights))},
          ~onHealthCheckResult=_ => (),
          params.languageInfo,
          params.setup,
        );

      dispatch(params.onStart(client));
      client;
    };

    let update = (~params as _, ~state, ~dispatch as _) => state;

    let dispose = (~params as _, ~state) => {
      let () = Oni_Syntax_Client.close(state);
      ();
    };
  });

let start = (languageInfo: Ext.LanguageInfo.t) => {

  let bufferEnterEffect = (maybeSyntaxClient, id: int, fileType) =>
    Isolinear.Effect.create(~name="syntax.bufferEnter", () => {
      OptionEx.iter2(
        (syntaxClient, fileType) => {
          Oni_Syntax_Client.notifyBufferEnter(syntaxClient, id, fileType)
        },
        maybeSyntaxClient,
        fileType,
      )
    });

  let bufferUpdateEffect =
      (
        maybeSyntaxClient,
        bufferUpdate: Oni_Core.BufferUpdate.t,
        lines,
        scopeMaybe,
      ) =>
    Isolinear.Effect.create(~name="syntax.bufferUpdate", () => {
      OptionEx.iter2(
        (syntaxClient, scope) => {
          Oni_Syntax_Client.notifyBufferUpdate(
            syntaxClient,
            bufferUpdate,
            lines,
            scope,
          )
        },
        maybeSyntaxClient,
        scopeMaybe,
      )
    });

  let configurationChangeEffect =
      (maybeSyntaxClient, config: Core.Configuration.t) =>
    Isolinear.Effect.create(~name="syntax.configurationChange", () => {
      Option.iter(
        syntaxClient =>
          Oni_Syntax_Client.notifyConfigurationChanged(syntaxClient, config),
        maybeSyntaxClient,
      )
    });

  let themeChangeEffect = (maybeSyntaxClient, theme) =>
    Isolinear.Effect.create(~name="syntax.theme", () => {
      Option.iter(
        syntaxClient => {
          Oni_Syntax_Client.notifyThemeChanged(syntaxClient, theme)
        },
        maybeSyntaxClient,
      )
    });

  let visibilityChangedEffect = (maybeSyntaxClient, visibleRanges) =>
    Isolinear.Effect.create(~name="syntax.visibilityChange", () => {
      Option.iter(
        syntaxClient =>
          Oni_Syntax_Client.notifyVisibilityChanged(
            syntaxClient,
            visibleRanges,
          ),
        maybeSyntaxClient,
      )
    });

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

  let updater = (state: Model.State.t, action) => {
    let default = (state, Isolinear.Effect.none);
    switch (action) {
    | Model.Actions.Init({syntaxHighlightingEnabled, _}) => (
        {...state, syntaxHighlightingEnabled},
        Isolinear.Effect.none,
      )
    | Model.Actions.SyntaxServerClosed => (
        {...state, syntaxClient: None},
        Isolinear.Effect.none,
      )
    | Model.Actions.SyntaxServerStarted(client) => (
        {...state, syntaxClient: Some(client)},
        Isolinear.Effect.none,
      )
    | Model.Actions.ReallyQuitting => (
        {...state, syntaxHighlightingEnabled: false},
        Isolinear.Effect.none,
      )
    | Model.Actions.ConfigurationSet(config) => (
        state,
        configurationChangeEffect(state.syntaxClient, config),
      )
    | Model.Actions.SetTokenTheme(tokenTheme) => (
        state,
        themeChangeEffect(state.syntaxClient, tokenTheme),
      )
    | Model.Actions.BufferEnter(metadata, fileType) =>
      let visibleBuffers =
        Model.EditorVisibleRanges.getVisibleBuffersAndRanges(state);

      let combinedEffects =
        Isolinear.Effect.batch([
          visibilityChangedEffect(state.syntaxClient, visibleBuffers),
          bufferEnterEffect(
            state.syntaxClient,
            Vim.BufferMetadata.(metadata.id),
            fileType,
          ),
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
      (state, visibilityChangedEffect(state.syntaxClient, visibleBuffers));
    // When there is a buffer update, send it over to the syntax highlight
    // strategy to handle the parsing.
    | Model.Actions.BufferUpdate({ update as bu, newBuffer, _}) =>
      let lines = Core.Buffer.getLines(newBuffer);
      let version = Core.Buffer.getVersion(newBuffer);
      let scope = getScopeForBuffer(newBuffer);
      if (!isVersionValid(version, bu.version)) {
        default;
      } else {
        (state, bufferUpdateEffect(state.syntaxClient, bu, lines, scope));
      };
    | _ => default
    };
  };

  updater;
};
