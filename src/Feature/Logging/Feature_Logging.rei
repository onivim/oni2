open Oni_Core;

// MODEL

type model;

let initial: model;

[@deriving show]
type msg;

// UPDATE

type outmsg =
  | Effect(Isolinear.Effect.t(msg))
  | ShowInfoNotification(string);

let update: (msg, model) => (model, outmsg);

// CONTRIBUTIONS

module Contributions: {let commands: list(Command.t(msg));};
