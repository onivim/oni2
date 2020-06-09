open Oni_Core;

type model;

let initial: model;

[@deriving show]
type command =
  | FormatDocument;

[@deriving show]
type msg =
  | Command(command)
  | DocumentFormatterAvailable({
    handle: int,
    selector: Exthost.DocumentSelector.t,
    displayName: string,
  });

let update: (model, msg) => model;

// COMMANDS

module Commands: {let formatDocument: Command.t(msg);};

// CONTRIBUTIONS

module Contributions: {let commands: list(Command.t(msg));};
