type oniCommand = {
  name: string,
<<<<<<< HEAD
<<<<<<< HEAD
  command: unit => list(Actions.t),
=======
  command: unit => Actions.t,
>>>>>>> Add input control mode to state
=======
  command: unit => list(Actions.t),
>>>>>>> master
};

type t = list(oniCommand);

let oniCommands = [
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> master
  {
    name: "commandPalette.open",
    command: _ => [
      CommandPaletteOpen,
      SetInputControlMode(CommandPaletteFocus),
    ],
  },
  {
    name: "commandPalette.close",
    command: _ => [
      CommandPaletteClose,
      SetInputControlMode(EditorTextFocus),
    ],
  },
  {name: "commandPalette.next", command: _ => [CommandPalettePosition(1)]},
  {
    name: "commandPalette.previous",
    command: _ => [CommandPalettePosition(-1)],
  },
  {
    name: "commandPalette.select",
    command: _ => [
      CommandPaletteSelect,
      SetInputControlMode(EditorTextFocus),
    ],
  },
<<<<<<< HEAD
=======
  {name: "open.commandPalette", command: () => CommandPaletteOpen},
  {name: "close.commandPalette", command: () => CommandPaletteClose},
>>>>>>> Add input control mode to state
=======
>>>>>>> master
];

let handleCommand = (~commands=oniCommands, name) => {
  let matchingCmd = List.find_opt(cmd => name == cmd.name, commands);
  switch (matchingCmd) {
  | Some(c) => c.command()
<<<<<<< HEAD
<<<<<<< HEAD
  | None => [Noop]
=======
  | None => Noop
>>>>>>> Add input control mode to state
=======
  | None => [Noop]
>>>>>>> master
  };
};
