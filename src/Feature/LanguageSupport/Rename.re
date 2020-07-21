open Oni_Core;
open Exthost;

[@deriving show]
type command =
  | RenameSymbol;

[@deriving show]
type msg =
  | Command(command);

type provider = {
  handle: int,
  selector: DocumentSelector.t,
  supportsResolveInitialValues: bool,
};

type model = {providers: list(provider)};

let initial = {providers: []};

let register = (~handle, ~selector, ~supportsResolveInitialValues, model) => {
  providers: [
    {handle, selector, supportsResolveInitialValues},
    ...model.providers,
  ],
};

let unregister = (~handle, model) => {
  providers:
    model.providers |> List.filter(provider => provider.handle != handle),
};

let update = (msg, model) => {
  switch (msg) {
  | Command(RenameSymbol) =>
    prerr_endline("is it wired up?");
    failwith("done!");
  };
};

module Commands = {
  open Feature_Commands.Schema;

  let rename =
    define(
      ~category="Language Support",
      ~title="Rename Symbol",
      "editor.action.rename",
      Command(RenameSymbol),
    );
};

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  // TODO:
  let renameInputVisible = bool("renameInputVisible", (_: model) => false);
};

module Keybindings = {
  open Oni_Input.Keybindings;

  let condition = "normalMode" |> WhenExpr.parse;

  let rename = {key: "<F2>", command: Commands.rename.id, condition};
};

module Contributions = {
  let commands = [Commands.rename];
  let contextKeys = [ContextKeys.renameInputVisible];
  let keybindings = [Keybindings.rename];
};
