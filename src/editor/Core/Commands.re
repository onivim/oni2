type oniCommand = {
  name: string,
  command: unit => list(Actions.t),
};

type t = list(oniCommand);

let oniCommands = [
  {
    name: "commandPalette.open",
    command: () => [CommandPaletteOpen, SetInputControlMode(Oni)],
  },
  {
    name: "commandPalette.close",
    command: () => [CommandPaletteClose, SetInputControlMode(Neovim)],
  },
  {name: "commandPalette.next", command: () => [CommandPalettePosition(1)]},
  {
    name: "commandPalette.previous",
    command: () => [CommandPalettePosition(-1)],
  },
];

let handleCommand = (~commands=oniCommands, name) => {
  let matchingCmd = List.find_opt(cmd => name == cmd.name, commands);
  switch (matchingCmd) {
  | Some(c) => c.command()
  | None => [Noop]
  };
};
