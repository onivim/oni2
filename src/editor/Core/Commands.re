type oniCommand = {
  name: string,
  command: unit => Actions.t,
};

type t = list(oniCommand);

let oniCommands = [
  {name: "commandPalette.open", command: () => CommandPaletteToggle(true)},
  {name: "commandPalette.close", command: () => CommandPaletteToggle(false)},
  {name: "commandPalette.next", command: () => CommandPalettePosition(1)},
  {
    name: "commandPalette.previous",
    command: () => CommandPalettePosition(-1),
  },
];

let handleCommand = (~commands=oniCommands, name) => {
  let matchingCmd = List.find_opt(cmd => name == cmd.name, commands);
  switch (matchingCmd) {
  | Some(c) => c.command()
  | None => Noop
  };
};
