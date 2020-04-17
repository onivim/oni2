module Core = Oni_Core;
module Syntax = Oni_Syntax;
module Ext = Oni_Extensions;
module OptionEx = Core.Utility.OptionEx;

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
  };

  module SyntaxSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = msg;

      type nonrec params = params;

      type state = {
        client: Oni_Syntax_Client.t,
        lastSyncedTokenTheme: option(Syntax.TokenTheme.t),
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
        { client, lastSyncedTokenTheme: None };
      };

      let update = (~params, ~state, ~dispatch as _) => {
         if (Some(params.tokenTheme) != state.lastSyncedTokenTheme) {
            Oni_Syntax_Client.notifyThemeChanged(state.client, params.tokenTheme);
            {
              ...state,
              lastSyncedTokenTheme: Some(params.tokenTheme),
            };
         } else {
            state;
         }
      }

      let dispose = (~params as _, ~state) => {
        let () = Oni_Syntax_Client.close(state.client);
        ();
      };
    });

  let create = (~languageInfo, ~setup, ~tokenTheme) => {
    SyntaxSubscription.create({id: "syntax-highligher", languageInfo, setup, tokenTheme});
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

  let configurationChange = (maybeSyntaxClient, config: Core.Configuration.t) =>
    Isolinear.Effect.create(~name="syntax.configurationChange", () => {
      Option.iter(
        syntaxClient =>
          Oni_Syntax_Client.notifyConfigurationChanged(syntaxClient, config),
        maybeSyntaxClient,
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
