type model = {isActive: bool};

let initial = {isActive: false};

let isActive = ({isActive}) => isActive;

module Register = {
  type t = char;

  // TODO: Validation
  let fromChar = v => Some(v);

  let toChar = v => v;
};

[@deriving show]
type command =
  | InsertRegister;

[@deriving show]
type msg =
  | RegisterNotAvailable
  | RegisterAvailable({contents: string})
  | Command(command);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | EmitRegister({
      contents: string,
      register: Register.t,
    });

let update = (msg, model) => {
  switch (msg) {
  | Command(InsertRegister) =>
    let toMsg = (
      fun
      | None => RegisterNotAvailable
      | Some(text) => RegisterAvailable({contents: text})
    );
    let eff = Service_Vim.Effects.getRegisterValue(~toMsg, 'a');
    (model, Effect(eff));
  | RegisterAvailable({contents}) => (
      model,
      EmitRegister({contents, register: 'a'}),
    )
  | RegisterNotAvailable => (model, Nothing)
  };
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
