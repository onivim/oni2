open Types;

type t = Palette.t;
open Palette;

let openConfigurationFile = _effects => {
  let sep = Rench.Path.pathSeparator;
  let _path =
    Revery.Environment.getExecutingDirectory()
    ++ sep
    ++ "configuration"
    ++ sep
    ++ "configuration.json";
  /* Core.Gl */
  ();
};

let commandPaletteCommands = effects => [
  {name: "Open configuration file", command: () => ()},
  {
    name: "Open keybindings file",
    command: () => openConfigurationFile(effects),
  },
];

let create = (~effects=?, ()) =>
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
    let selected = List.nth(state.commands, state.selectedItem);
    selected.command();
    state;
  | _ => state
  };
