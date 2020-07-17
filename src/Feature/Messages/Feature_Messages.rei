open Exthost.Message;

[@deriving show]
type msg;

module Msg: {
    let exthost:
    (~dispatch: (msg) => unit,
    Exthost.Msg.MessageService.msg) => Lwt.t(option(Exthost.Message.handle));
};

type model;

let initial: model;

type outmsg =
| Nothing
| Effect(Isolinear.Effect.t(msg));

let update: (msg, model) => (model, outmsg);
