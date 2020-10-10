open Oni_Core;

type outmsg =
  | Nothing
  | DebugInputShown;

[@deriving show]
type command;

[@deriving show]
type msg;

type model;

let initial: model;

let update: (msg, model) => (model, outmsg);

module Contributions: {let commands: list(Command.t(msg));};
