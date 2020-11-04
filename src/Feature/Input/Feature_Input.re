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
  | Command(command)
  | VimMap(Vim.Mapping.t)
  | VimUnmap({
      mode: Vim.Mapping.mode,
      maybeKeys: option(string),
    });

module Msg = {
  let vimMap = mapping => VimMap(mapping);
  let vimUnmap = (mode, maybeKeys) => VimUnmap({mode, maybeKeys});
};

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
module Internal = {
  let vimMapModeToWhenExpr = mode => {
    let parse = str => WhenExpr.parse(str);
    let evaluateCondition = (whenExpr, contextKeys) => {
      WhenExpr.evaluate(
        whenExpr,
        WhenExpr.ContextKeys.getValue(contextKeys),
      );
    };
    let condition =
      mode
      |> Vim.Mapping.(
           {
             fun
             | Insert => "insertMode" |> parse
             | Language => WhenExpr.Value(False) // TODO
             | CommandLine => "commandLineFocus" |> parse
             | Normal => "normalMode" |> parse
             | VisualAndSelect => "selectMode || visualMode" |> parse
             | Visual => "visualMode" |> parse
             | Select => "selectMode" |> parse
             | Operator => "operatorPending" |> parse
             | Terminal => "terminalFocus" |> parse
             | InsertAndCommandLine =>
               "insertMode || commandLineFocus" |> parse
             | All => WhenExpr.Value(True);
           }
         );

    evaluateCondition(condition);
  };
};

let update = (msg, model) => {
  switch (msg) {
  | Command(ShowDebugInput) => (model, DebugInputShown)
  | VimMap(mapping) =>
    let matcher =
      EditorInput.Matcher.parse(
        ~getKeycode,
        ~getScancode,
        mapping.fromKeys,
        // TODO: Fix this
      )
      |> Result.get_ok;
    let keys =
      EditorInput.KeyPress.parse(
        ~getKeycode,
        ~getScancode,
        mapping.toValue,
        // TODO: Fix this
      )
      |> Result.get_ok;
    let (inputStateMachine', _mappingId) =
      InputStateMachine.addMapping(
        matcher,
        // TODO: Convert mode to WhenExpr
        Internal.vimMapModeToWhenExpr(mapping.mode),
        keys,
        model.inputStateMachine,
      );
    ({inputStateMachine: inputStateMachine'}, Nothing);
  | VimUnmap({mode, maybeKeys}) => (model, Nothing)
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
