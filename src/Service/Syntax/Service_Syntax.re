module Core = Oni_Core;
module Ext = Oni_Extensions;
open Core.Utility;

type msg =
  | ServerStarted(Oni_Syntax_Client.t)
  | ServerClosed
  | ReceivedHighlights(list(Oni_Syntax.Protocol.TokenUpdate.t));

module Sub = {
  type params = {
    id: string,
    languageInfo: Ext.LanguageInfo.t,
    setup: Core.Setup.t,
  };

  module SyntaxSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = msg;

      type nonrec params = params;

      type state = Oni_Syntax_Client.t;

      let subscriptionName = "SyntaxSubscription";
      let getUniqueId = params => params.id;

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
        client;
      };

      let update = (~params as _, ~state, ~dispatch as _) => state;

      let dispose = (~params as _, ~state) => {
        let () = Oni_Syntax_Client.close(state);
        ();
      };
    });

  let create = (languageInfo, setup) => {
    SyntaxSubscription.create({id: "syntax-highligher", languageInfo, setup});
  };
};
