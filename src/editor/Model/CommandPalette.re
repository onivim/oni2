open Oni_Core;
open Types;

open UiMenu;

let openConfigurationFile = (effects: Effects.t(Actions.t), name) =>
  switch (Filesystem.createOniConfigFile(name)) {
  | Ok(path) => effects.openFile(~path, ())
  | Error(e) => print_endline(e)
  };

let commands = (effects: Effects.t(Actions.t)) => [
  {
    name: "Open configuration file",
    command: () => openConfigurationFile(effects, "configuration.json"),
    icon: None,
  },
  {
    name: "Open keybindings file",
    command: () => openConfigurationFile(effects, "keybindings.json"),
    icon: None,
  },
];
