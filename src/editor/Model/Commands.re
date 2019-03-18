type oniCommand = {
  name: string,
  command: unit => list(Actions.t),
};

type t = list(oniCommand);

let oniCommands = [
  {
    name: "commandPalette.open",
    command: _ => [
      MenuOpen((CommandPalette, CommandPalette.commandPaletteCommands)),
      SetInputControlMode(CommandPaletteFocus),
    ],
  },
  {
    name: "commandPalette.close",
    command: _ => [MenuClose, SetInputControlMode(EditorTextFocus)],
  },
  {name: "commandPalette.next", command: _ => [MenuPosition(1)]},
  {name: "commandPalette.previous", command: _ => [MenuPosition(-1)]},
  {
    name: "commandPalette.select",
    command: _ => [MenuSelect, SetInputControlMode(EditorTextFocus)],
  },
];

let handleCommand = (~commands=oniCommands, name) => {
  let matchingCmd = List.find_opt(cmd => name == cmd.name, commands);
  switch (matchingCmd) {
  | Some(c) => c.command()
  | None => [Noop]
  };
};
