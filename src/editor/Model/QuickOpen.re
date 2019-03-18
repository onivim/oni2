open Oni_Core.Types;

let content = (effects: Effects.t): list(UiMenu.command) => [
  {name: "test.txt", command: () => effects.openFile(~path="test.txt", ())},
];
