type model = {isActive: bool};

let initial = {isActive: false};

let isActive = ({isActive}) => isActive;

module Register = {
  type t = char;

  // TODO: Validation
  let fromChar = v => Some(v);

  let toChar = v => v;
};

type outmsg =
  | Nothing
  | Focus
  | EmitRegister({
      contents: string,
      register: Register.t,
    });

type command =
  | InsertRegister;

type msg =
  | Command(command);

let update = (msg, model) => {
  (model, Nothing);
};

module Commands = {
  open Feature_Commands.Schema;

  let insert =
    define(
      ~category="Vim",
      ~title="Insert register",
      "vim.insertRegister",
      Command(InsertRegister),
    );
};

module Contributions = {
  let commands = [Commands.insert];
};
