type oniCommand = {
  name: string,
  command: unit => list(Actions.t),
};

type t = list(oniCommand);

let oniCommands = [
  {
    name: "commandPalette.open",
    command: _ => [CommandPaletteOpen, SetInputControlMode(Palette)],
  },
  {
    name: "commandPalette.close",
    command: _ => [CommandPaletteClose, SetInputControlMode(Neovim)],
  },
  {name: "commandPalette.next", command: _ => [CommandPalettePosition(1)]},
  {
    name: "commandPalette.previous",
    command: _ => [CommandPalettePosition(-1)],
  },
  {
    name: "commandPalette.select",
    command: _ => [CommandPaletteSelect, SetInputControlMode(Neovim)],
  },
];

let handleCommand = (~commands=oniCommands, name) => {
  let matchingCmd = List.find_opt(cmd => name == cmd.name, commands);
  switch (matchingCmd) {
  | Some(c) => c.command()
  | None => [Noop]
  };
};
