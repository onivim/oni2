open Types.Input;

type t = list(keyBindings);

let defaultCommands = [
  {key: "<C-P>", command: "commandPalette.open", condition: Neovim},
  {key: "<ESC>", command: "commandPalette.close", condition: Oni},
  {key: "<C-N>", command: "commandPalette.next", condition: Oni},
  {key: "<C-P>", command: "commandPalette.previous", condition: Oni},
];
