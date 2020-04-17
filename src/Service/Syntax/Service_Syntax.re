module Core = Oni_Core;
module Syntax = Oni_Syntax;
module Ext = Oni_Extensions;
module OptionEx = Core.Utility.OptionEx;

module Log = (val Core.Log.withNamespace("Oni2.Service_Syntax"));

[@deriving show({with_path: false})]
type msg =
  | ServerStarted([@opaque] Oni_Syntax_Client.t)
  | ServerFailedToStart(string)
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
        client: result(Oni_Syntax_Client.t, string),
        lastSyncedTokenTheme: option(Syntax.TokenTheme.t),
        lastConfiguration: option(Core.Configuration.t),
      };

      let name = "SyntaxSubscription";
      let id = params => params.id;

      let init = (~params, ~dispatch) => {
        Log.info("Init called");
        let pendingResult = ref(None);
        let clientResult =
          Oni_Syntax_Client.start(
            ~onConnected={
            () => {
        Log.info("onConnected");
            pendingResult^ |> Option.iter(server => dispatch(ServerStarted(server)));
            }},
            ~onClose=_ => dispatch(ServerClosed),
            ~onHighlights=
              highlights => {dispatch(ReceivedHighlights(highlights))},
            ~onHealthCheckResult=_ => (),
            params.languageInfo,
            params.setup,
          )
          |> Utility.ResultEx.tap(server => dispatch(ServerStarted(server)))
          |> Utility.ResultEx.tapError(msg =>
               dispatch(ServerFailedToStart(msg))
             );

        {client: clientResult, lastSyncedTokenTheme: None, lastConfiguration: None};
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
          state.client
          |> Result.map((client) => {
            Oni_Syntax_Client.notifyThemeChanged(client, tokenTheme);
            {...state, lastSyncedTokenTheme: Some(tokenTheme)};
          })
          |> Result.value(~default=state);
        } else {
          state;
        };

      let syncConfiguration = (configuration, state) =>
        if (!compare(configuration, state.lastConfiguration)) {
          state.client
          |> Result.map(client => {
          Oni_Syntax_Client.notifyConfigurationChanged(
            client,
            configuration,
          );
          {...state, lastConfiguration: Some(configuration)};
  
          })
          |> Result.value(~default=state);
        } else {
          state;
        };

      let update = (~params, ~state, ~dispatch as _) => {
        state
        |> syncTokenTheme(params.tokenTheme)
        |> syncConfiguration(params.configuration);
      };

      let dispose = (~params as _, ~state) => {
        state.client |> Result.iter(Oni_Syntax_Client.close);
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
