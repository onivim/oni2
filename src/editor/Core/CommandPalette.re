open Types;

type t = Palette.t;
open Palette;

let openConfigurationFile = (effects: Effects.t) => {
  let path =
    Utility.join([
      Revery.Environment.getWorkingDirectory(),
      "assets",
      "configuration",
      "configuration.json",
    ]);
  switch (Filesystem.createOniDirectory()) {
  | Ok(_) => effects.openFile(~path, ())
  | Error(e) => print_endline(e)
  };
};

let openKeybindingsFile = (effects: Effects.t) => {
  let path =
    Utility.join([
      Revery.Environment.getWorkingDirectory(),
      "assets",
      "configuration",
      "keybindings.json",
    ]);
  effects.openFile(~path, ());
};

let commandPaletteCommands = (effects: Effects.t) => [
  {
    name: "Open configuration file",
    command: () => openConfigurationFile(effects),
  },
  {
    name: "Open keybindings file",
    command: () => openKeybindingsFile(effects),
  },
];

let create = (~effects: option(Effects.t)=?, ()) =>
  switch (effects) {
  | Some(e) => {
      isOpen: false,
      commands: commandPaletteCommands(e),
      selectedItem: 0,
    }
  | None => {isOpen: false, commands: [], selectedItem: 0}
  };

let make = (~effects: Effects.t) =>
  create(~effects, ()) |> (c => Actions.CommandPaletteStart(c.commands));

let position = (selectedItem, change, commands: list(command)) =>
  selectedItem + change >= List.length(commands) ? 0 : selectedItem + change;

let reduce = (state: t, action: Actions.t) =>
  switch (action) {
  | CommandPaletteStart(commands) => {...state, commands}
  | CommandPalettePosition(pos) => {
      ...state,
      selectedItem: position(state.selectedItem, pos, state.commands),
    }
  | CommandPaletteOpen => {...state, isOpen: true}
  | CommandPaletteClose => {...state, isOpen: false}
  | CommandPaletteSelect =>
    /**
       TODO: Refactor this to middleware so this action is handled like a redux side-effect
       as this makes the reducer impure
     */
    let selected = List.nth(state.commands, state.selectedItem);
    selected.command();
    {...state, isOpen: false};
  | _ => state
  };
