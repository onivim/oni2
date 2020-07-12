open Oni_Core;

type model;
let initial: model;

[@deriving show]
type msg;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | Pasted({
      rawText: string,
      isMultiLine: bool,
      lines: array(string),
    });

let update: (msg, model) => (model, outmsg);

module Commands: {let paste: Command.t(msg);};

module Contributions: {let commands: list(Command.t(msg));};
