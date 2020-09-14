// MODEL

module Decoration: {
  [@deriving show]
  type t = {
    handle: int, // provider handle
    tooltip: string,
    letter: string,
    color: string // TODO: ThemeColor.t?
  };
};

[@deriving show]
type msg;

module Msg: {let exthost: Exthost.Msg.Decorations.msg => msg;};

type model;

let initial: model;

let getDecorations: (~path: string, model) => list(Decoration.t);

// UPDATE

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let update: (~client: Exthost.Client.t, msg, model) => (model, outmsg);
