module Core = Oni_Core;
module Syntax = Oni_Syntax;
module Ext = Oni_Extensions;
module OptionEx = Core.Utility.OptionEx;

module Log = (val Core.Log.withNamespace("Oni2.Service_Syntax"));

[@deriving show({with_path: false})]
type msg =
  | ServerStarted([@opaque] Oni_Syntax_Client.t)
  | ServerClosed
  | ReceivedHighlights([@opaque] list(Oni_Syntax.Protocol.TokenUpdate.t));

module Sub = {
  type params = {
    id: string,
    languageInfo: Ext.LanguageInfo.t,
    setup: Core.Setup.t,
    tokenTheme: Syntax.TokenTheme.t,
    configuration: Core.Configuration.t,
  };

  module SyntaxSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = msg;

      type nonrec params = params;

      type state = {
        client: Oni_Syntax_Client.t,
        lastSyncedTokenTheme: option(Syntax.TokenTheme.t),
        lastConfiguration: option(Core.Configuration.t),
      };

      let name = "SyntaxSubscription";
      let id = params => params.id;

      let init = (~params, ~dispatch) => {
        let client =
          Oni_Syntax_Client.start(
            ~onClose=_ => dispatch(ServerClosed),
            ~scheduler=Core.Scheduler.mainThread,
            ~onHighlights=
              highlights => {dispatch(ReceivedHighlights(highlights))},
            ~onHealthCheckResult=_ => (),
            params.languageInfo,
            params.setup,
          );

        dispatch(ServerStarted(client));
        {client, lastSyncedTokenTheme: None, lastConfiguration: None};
      };

      let compare: ('a, option('a)) => bool =
        (v, opt) => {
          switch (opt) {
          | None => false
          | Some(innerVal) => innerVal === v
          };
        };

      let syncTokenTheme = (tokenTheme, state) =>
        if (!compare(tokenTheme, state.lastSyncedTokenTheme)) {
          Oni_Syntax_Client.notifyThemeChanged(state.client, tokenTheme);
          {...state, lastSyncedTokenTheme: Some(tokenTheme)};
        } else {
          state;
        };

      let syncConfiguration = (configuration, state) =>
        if (!compare(configuration, state.lastConfiguration)) {
          Oni_Syntax_Client.notifyConfigurationChanged(
            state.client,
            configuration,
          );
          {...state, lastConfiguration: Some(configuration)};
        } else {
          state;
        };

      let update = (~params, ~state, ~dispatch as _) => {
        state
        |> syncTokenTheme(params.tokenTheme)
        |> syncConfiguration(params.configuration);
      };

      let dispose = (~params as _, ~state) => {
        let () = Oni_Syntax_Client.close(state.client);
        ();
      };
    });

  let create = (~configuration, ~languageInfo, ~setup, ~tokenTheme) => {
    SyntaxSubscription.create({
      id: "syntax-highligher",
      configuration,
      languageInfo,
      setup,
      tokenTheme,
    });
  };
};

module Effect = {
  let bufferEnter = (maybeSyntaxClient, id: int, fileType) =>
    Isolinear.Effect.create(~name="syntax.bufferEnter", () => {
      OptionEx.iter2(
        (syntaxClient, fileType) => {
          Oni_Syntax_Client.notifyBufferEnter(syntaxClient, id, fileType)
        },
        maybeSyntaxClient,
        fileType,
      )
    });

  let bufferUpdate =
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

  let visibilityChanged = (maybeSyntaxClient, visibleRanges) =>
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
};
