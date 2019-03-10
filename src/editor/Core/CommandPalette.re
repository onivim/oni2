type command = {
  name: string,
  command: unit => Actions.t,
};

type t = {
  isOpen: bool,
  commands: list(command),
  selectedItem: int,
};

let commandPaletteCommands = [
  {name: "testing1", command: () => Noop},
  {name: "testing2", command: () => Noop},
  {name: "testing3", command: () => Noop},
];

let setInputControl = isOpen => Types.Input.(isOpen ? Oni : Neovim);

let create = () => {
  isOpen: false,
  commands: commandPaletteCommands,
  selectedItem: 0,
};

let position = (selectedItem, change, commands: list(command)) =>
  selectedItem + change >= List.length(commands) ? 0 : selectedItem + change;

let reduce = (state: t, action: Actions.t) =>
  switch (action) {
  | CommandPalettePosition(pos) => {
      ...state,
      selectedItem: position(state.selectedItem, pos, state.commands),
    }
  | CommandPaletteOpen => {...state, isOpen: true}
  | CommandPaletteClose => {...state, isOpen: false}
  | _ => state
  };
