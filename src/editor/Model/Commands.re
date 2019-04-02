type oniCommand = {
  name: string,
  command: unit => list(Actions.t),
};

type t = list(oniCommand);

let oniCommands = [
  {
    name: "commandPalette.open",
    command: _ => [
      MenuOpen(CommandPalette.create),
      SetInputControlMode(TextInputFocus),
    ],
  },
  {name: "quickOpen.open", command: _ => [QuickOpen]},
  {
    name: "menu.close",
    command: _ => [MenuClose, SetInputControlMode(EditorTextFocus)],
  },
  {name: "menu.next", command: _ => [MenuPosition(1)]},
  {name: "menu.previous", command: _ => [MenuPosition(-1)]},
  {
    name: "menu.next",
    command: _ => [SetInputControlMode(MenuFocus), MenuPosition(1)],
  },
  {
    name: "menu.previous",
    command: _ => [SetInputControlMode(MenuFocus), MenuPosition(-1)],
  },
  {
    name: "menu.select",
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
