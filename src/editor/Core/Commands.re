type oniCommand = {
  name: string,
  command: unit => Actions.t,
};

type t = list(oniCommand);

let oniCommands = [
  {name: "open.commandPalette", command: () => CommandPaletteOpen},
  {name: "close.commandPalette", command: () => CommandPaletteClose},
];

let handleCommand = (~commands=oniCommands, name) => {
  let matchingCmd = List.find_opt(cmd => name == cmd.name, commands);
  switch (matchingCmd) {
  | Some(c) => c.command()
  | None => Noop
  };
};
