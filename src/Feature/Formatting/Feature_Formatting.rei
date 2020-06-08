open Oni_Core;

type model;

let initial: model;

[@deriving show]
type msg;

module Commands: {let formatDocument: Command.t(msg);};

module Contributions: {let commands: list(Command.t(msg));};
