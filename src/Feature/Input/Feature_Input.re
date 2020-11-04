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

type model = {
  inputStateMachine: InputStateMachine.t,
};

let initial = {
  inputStateMachine: InputStateMachine.empty,
};

type effect = InputStateMachine.effect =
| Execute(string)
| Text(string)
| Unhandled(EditorInput.KeyPress.t)
| RemapRecursionLimitHit;

let keyDown = (~key, ~context, {inputStateMachine, _}) => {
  let (inputStateMachine', effects) = InputStateMachine.keyDown(
    ~key,
    ~context,
  inputStateMachine);
  ({inputStateMachine: inputStateMachine'}, effects);
}

let text = (~text, {inputStateMachine, _}) => {
  let (inputStateMachine', effects) = InputStateMachine.text(
    ~text,
  inputStateMachine);
  ({inputStateMachine: inputStateMachine'}, effects);
}

let keyUp = (~key, ~context, {inputStateMachine, _}) => {
  let (inputStateMachine', effects) = InputStateMachine.keyUp(
    ~key,
    ~context,
  inputStateMachine);
  ({inputStateMachine: inputStateMachine'}, effects);
}

// UPDATE

let update = (msg, model) => {
  switch (msg) {
  | Command(ShowDebugInput) => (model, DebugInputShown)
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
