type model;

let initial: model;

type outmsg =
  | Nothing;

[@deriving show]
type msg;

module Msg: {let exthost: Exthost.Msg.QuickOpen.msg => msg;};

let update: (msg, model) => (model, outmsg);
