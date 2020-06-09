type model;

let initial: model;

[@deriving show({with_path: false})]
type command;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | KeyPressed(string);

let update: (model, msg) => model;

module Contributions: {let commands: list(Oni_Core.Command.t(msg));};

module View: {
  let make:
    (~theme: Oni_Core.ColorTheme.Colors.t, ~model: model, unit) =>
    Revery.UI.element;
};
