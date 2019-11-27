/*
 * Commands.re
 *
 * Type definitions for commands.
 */

open Oni_Extensions;

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

let ofExtensions = (extensions: list(ExtensionScanner.t)) => {
  open ExtensionContributions;
  open ExtensionContributions.Commands;

  let getContributedCommands = (v: ExtensionScanner.t) =>
    v.manifest.contributes.commands;

  let toCommand = (v: ExtensionContributions.Commands.t) => {
    Command.create(
      ~name=v.title,
      ~action=Actions.CommandExecuteContributed(v.command),
      ~category=v.category,
      (),
    );
  };

  extensions
  |> List.map(getContributedCommands)
  |> List.flatten
  |> List.map(toCommand);
};

let reduce = (v: t, action: Actions.t) =>
  switch (action) {
  | Actions.CommandsRegister(cmds) => v @ cmds
  | _ => v
  };
