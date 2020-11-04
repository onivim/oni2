open KeyResolver;

open Oni_Core;
module Log = (val Log.withNamespace("Oni2.Feature.Input"));

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

module Schema = {
  [@deriving show]
  type keybinding = {
    key: string,
    command: string,
    condition: WhenExpr.t,
  };

  type resolvedKeybinding = {
    matcher: EditorInput.Matcher.t,
    command: string,
    condition: WhenExpr.ContextKeys.t => bool,
  };

  let resolve = ({key, command, condition}) => {
    let evaluateCondition = (whenExpr, contextKeys) => {
      WhenExpr.evaluate(
        whenExpr,
        WhenExpr.ContextKeys.getValue(contextKeys),
      );
    };

    let maybeMatcher =
      EditorInput.Matcher.parse(~getKeycode, ~getScancode, key);
    maybeMatcher
    |> Stdlib.Result.map(matcher => {
         {matcher, command, condition: evaluateCondition(condition)}
       });
  };
};

type model = {inputStateMachine: InputStateMachine.t};

let initial = keybindings => {
  open Schema;
  let inputStateMachine =
    keybindings
    |> List.fold_left(
         (ism, keybinding) => {
           switch (Schema.resolve(keybinding)) {
           | Error(err) =>
             Log.warnf(m =>
               m(
                 "Unable to parse binding: %s",
                 Schema.show_keybinding(keybinding),
               )
             );
             ism;
           | Ok({matcher, condition, command}) =>
             let (ism, _bindingId) =
               InputStateMachine.addBinding(matcher, condition, command, ism);
             ism;
           }
         },
         InputStateMachine.empty,
       );
  {inputStateMachine: inputStateMachine};
};

type effect =
  InputStateMachine.effect =
    | Execute(string)
    | Text(string)
    | Unhandled(EditorInput.KeyPress.t)
    | RemapRecursionLimitHit;

let keyDown = (~key, ~context, {inputStateMachine, _}) => {
  let (inputStateMachine', effects) =
    InputStateMachine.keyDown(~key, ~context, inputStateMachine);
  ({inputStateMachine: inputStateMachine'}, effects);
};

let text = (~text, {inputStateMachine, _}) => {
  let (inputStateMachine', effects) =
    InputStateMachine.text(~text, inputStateMachine);
  ({inputStateMachine: inputStateMachine'}, effects);
};

let keyUp = (~key, ~context, {inputStateMachine, _}) => {
  let (inputStateMachine', effects) =
    InputStateMachine.keyUp(~key, ~context, inputStateMachine);
  ({inputStateMachine: inputStateMachine'}, effects);
};

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
