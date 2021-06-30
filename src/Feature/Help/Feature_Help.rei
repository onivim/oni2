open Oni_Core;

type model;

let initial: model;

[@deriving show]
type msg;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let update: (msg, model) => (model, outmsg);

module Contributions: {
  let commands: list(Command.t(msg));
  let menuGroups: list(ContextMenu.Schema.group);
};
