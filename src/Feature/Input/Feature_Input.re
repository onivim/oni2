// MSG

type outmsg =
  | Nothing
  | DebugInputShown;

[@deriving show]
type command =
  | ShowDebugInput;

[@deriving show]
type msg =
  | Command(command);

// MODEL

type model = unit;

let initial = ();

// UPDATE

let update = (msg, _model) => {
  switch (msg) {
  | Command(ShowDebugInput) => ((), DebugInputShown)
  };
};

// COMMANDS

module Commands = {
  open Feature_Commands.Schema;

  let showInputState =
    define(
      ~category="Debug",
      ~title="Show input state",
      "oni2.debug.showInput",
      Command(ShowDebugInput),
    );
};

module Contributions = {
  let commands = Commands.[showInputState];
};
