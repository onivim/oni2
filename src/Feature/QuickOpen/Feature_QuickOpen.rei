type model;

let initial: model;

[@deriving show]
type msg;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | ShowMenu(Feature_Quickmenu.Schema.menu(msg));

module Msg: {
  let exthost:
    (~resolver: Lwt.u(Exthost.Reply.t), Exthost.Msg.QuickOpen.msg) => msg;
};

let update: (~client: Exthost.Client.t, msg, model) => (model, outmsg);
