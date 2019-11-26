/*
 * Commands.re
 *
 * Type definitions for commands.
 */

type t = list(Command.t);

let empty = [];

let add = (cmd: Command.t, v: t) => {
  [cmd, ...v];
};

let getEnabled = (v: t) => {
  let filter = (item: Command.t) => Command.isEnabled(item);
  List.filter(filter, v);
};

let toQuickMenu = (v: t) => {
  getEnabled(v) |> List.map(Command.toQuickMenu);
};

let reduce = (v: t, action: Actions.t) =>
  switch (action) {
  | Actions.CommandsRegister(cmds) => v @ cmds
  | _ => v
  };
