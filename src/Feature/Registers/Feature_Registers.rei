open Oni_Core;

module Register: {
  type t;

  let fromChar: char => option(t);
  let toChar: t => char;
};

type model;

let initial: model;

[@deriving show]
type msg;

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | EmitRegister({
      raw: string,
      lines: array(string),
      register: Register.t,
    });

let update: (msg, model) => (model, outmsg);

let isActive: model => bool;

module Commands: {let insert: Command.t(msg);};

module Contributions: {let commands: list(Command.t(msg));};
