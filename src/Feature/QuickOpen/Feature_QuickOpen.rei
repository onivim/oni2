type model;

let initial: model;

[@deriving show]
type msg;

type outmsg =
  | Nothing
  | ShowMenu(Feature_Quickmenu.Schema.menu(msg));

module Msg: {let exthost: Exthost.Msg.QuickOpen.msg => msg;};

let update: (msg, model) => (model, outmsg);
