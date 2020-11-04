open KeyResolver;

open Oni_Core;
open Utility;
module Log = (val Log.withNamespace("Oni2.Feature.Input"));

// MSG

type outmsg =
  | Nothing
  | DebugInputShown
  | MapParseError({
      fromKeys: string,
      toKeys: string,
      error: string,
    });

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

[@deriving show]
type command =
  | ShowDebugInput;

[@deriving show]
type msg =
  | Command(command)
  | KeybindingsUpdated([@opaque] list(Schema.resolvedKeybinding))
  | VimMap(Vim.Mapping.t)
  | VimUnmap({
      mode: Vim.Mapping.mode,
      maybeKeys: option(string),
    });

module Msg = {
  let keybindingsUpdated = keybindings => KeybindingsUpdated(keybindings);
  let vimMap = mapping => VimMap(mapping);
  let vimUnmap = (mode, maybeKeys) => VimUnmap({mode, maybeKeys});
};

// MODEL

type model = {
  userBindings: list(InputStateMachine.uniqueId),
  inputStateMachine: InputStateMachine.t,
};

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
                 "Unable to parse binding %s: %s",
                 Schema.show_keybinding(keybinding),
                 err,
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
  {inputStateMachine, userBindings: []};
};

type effect =
  InputStateMachine.effect =
    | Execute(string)
    | Text(string)
    | Unhandled(EditorInput.KeyPress.t)
    | RemapRecursionLimitHit;

let keyDown = (~key, ~context, {inputStateMachine, _} as model) => {
  let (inputStateMachine', effects) =
    InputStateMachine.keyDown(~key, ~context, inputStateMachine);
  ({...model, inputStateMachine: inputStateMachine'}, effects);
};

let text = (~text, {inputStateMachine, _} as model) => {
  let (inputStateMachine', effects) =
    InputStateMachine.text(~text, inputStateMachine);
  ({...model, inputStateMachine: inputStateMachine'}, effects);
};

let keyUp = (~key, ~context, {inputStateMachine, _} as model) => {
  let (inputStateMachine', effects) =
    InputStateMachine.keyUp(~key, ~context, inputStateMachine);
  ({...model, inputStateMachine: inputStateMachine'}, effects);
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

  let updateKeybindings = (newBindings, model) => {
    // First, clear all the old bindings...
    let inputStateMachine' =
      model.userBindings
      |> List.fold_left(
           (ism, bindingId) => {InputStateMachine.remove(bindingId, ism)},
           model.inputStateMachine,
         );

    // Then, add all new bindings
    let (inputStateMachine'', userBindingIds) =
      newBindings
      |> List.fold_left(
           (acc, resolvedBinding: Schema.resolvedKeybinding) => {
             let (ism, bindings) = acc;
             let (ism', bindingId) =
               InputStateMachine.addBinding(
                 resolvedBinding.matcher,
                 resolvedBinding.condition,
                 resolvedBinding.command,
                 ism,
               );
             (ism', [bindingId, ...bindings]);
           },
           (inputStateMachine', []),
         );

    {inputStateMachine: inputStateMachine'', userBindings: userBindingIds};
  };
};

let update = (msg, model) => {
  switch (msg) {
  | Command(ShowDebugInput) => (model, DebugInputShown)
  | VimMap(mapping) =>
    let maybeMatcher =
      EditorInput.Matcher.parse(~getKeycode, ~getScancode, mapping.fromKeys);
    let maybeKeys =
      EditorInput.KeyPress.parse(~getKeycode, ~getScancode, mapping.toValue);

    let maybeModel =
      ResultEx.map2(
        (matcher, keys) => {
          let (inputStateMachine', _mappingId) =
            InputStateMachine.addMapping(
              matcher,
              Internal.vimMapModeToWhenExpr(mapping.mode),
              keys,
              model.inputStateMachine,
            );
          {...model, inputStateMachine: inputStateMachine'};
        },
        maybeMatcher,
        maybeKeys,
      );

    switch (maybeModel) {
    | Ok(model') => (model', Nothing)
    | Error(err) => (
        model,
        MapParseError({
          fromKeys: mapping.fromKeys,
          toKeys: mapping.toValue,
          error: err,
        }),
      )
    };

  | VimUnmap(_) => (model, Nothing)
  // TODO:
  // | VimUnmap({mode, maybeKeys}) => (model, Nothing)

  | KeybindingsUpdated(bindings) => (
      Internal.updateKeybindings(bindings, model),
      Nothing,
    )
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
