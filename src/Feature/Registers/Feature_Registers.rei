open Oni_Core;

module Register: {
  type t;

  let fromChar: char => option(t);
  let toChar: t => char;
};

type model;

let initial: model;

type outmsg =
  | Nothing
  | Focus
  | EmitRegister({
      contents: string,
      register: Register.t,
    });

type msg;

let update: (msg, model) => (model, outmsg);

let isActive: model => bool;

module Commands: {let insert: Command.t(msg);};

module Contributions: {let commands: list(Command.t(msg));};
