open KeyResolver;

open Oni_Core;
open Utility;
module Log = (val Log.withNamespace("Oni2.Feature.Input"));

let keyCodeToString = Sdl2.Keycode.getName;

let keyPressToString =
  EditorInput.KeyPress.toString(~meta="Meta", ~keyCodeToString);

// CONFIGURATION
module Configuration = {
  open Oni_Core;
  open Config.Schema;

  module CustomDecoders = {
    let physicalKey =
      custom(
        ~decode=
          Json.Decode.(
            string
            |> and_then(keyString =>
                 if (keyString == "(none)") {
                   succeed(None);
                 } else {
                   switch (
                     EditorInput.KeyPress.parse(
                       // When parsing from JSON - use VSCode style parsing
                       // where an explicit shift key is required.
                       ~explicitShiftKeyNeeded=true,
                       ~getKeycode,
                       ~getScancode,
                       keyString,
                     )
                   ) {
                   | Ok([]) =>
                     fail("Unable to parse key sequence: " ++ keyString)
                   | Ok([key]) =>
                     switch (EditorInput.KeyPress.toPhysicalKey(key)) {
                     | None => fail("Not a physical key: " ++ keyString)
                     | Some(physicalKey) => succeed(Some(physicalKey))
                     }
                   | Ok(_keys) =>
                     fail(
                       "Unable to parse key sequence - too many keys: "
                       ++ keyString,
                     )
                   | Error(msg) =>
                     fail("Unable to parse key sequence: " ++ msg)
                   };
                 }
               )
          ),
        ~encode=
          Json.Encode.(
            maybeKey => {
              switch (maybeKey) {
              | Some(key) =>
                EditorInput.KeyPress.toString(
                  ~keyCodeToString=Sdl2.Keycode.getName,
                  EditorInput.KeyPress.PhysicalKey(key),
                )
                |> string
              | None => "(none)" |> string
              };
            }
          ),
      );
  };

  let leaderKey =
    setting("vim.leader", CustomDecoders.physicalKey, ~default=None);
};

// MSG

type outmsg =
  | Nothing
  | DebugInputShown
  | MapParseError({
      fromKeys: string,
      toKeys: string,
      error: string,
    });

type execute =
  InputStateMachine.execute = | NamedCommand(string) | VimExCommand(string);

module Schema = {
  [@deriving show]
  type keybinding = {
    key: string,
    command: string,
    condition: WhenExpr.t,
  };

  type resolvedKeybinding = {
    matcher: EditorInput.Matcher.t,
    command: InputStateMachine.execute,
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
      EditorInput.Matcher.parse(
        ~explicitShiftKeyNeeded=true,
        ~getKeycode,
        ~getScancode,
        key,
      );
    maybeMatcher
    |> Stdlib.Result.map(matcher => {
         {
           matcher,
           command: InputStateMachine.NamedCommand(command),
           condition: evaluateCondition(condition),
         }
       });
  };
};

[@deriving show]
type command =
  | ShowDebugInput
  | EnableKeyDisplayer
  | DisableKeyDisplayer;

[@deriving show]
type msg =
  | Command(command)
  | KeybindingsUpdated([@opaque] list(Schema.resolvedKeybinding))
  | VimMap(Vim.Mapping.t)
  | VimUnmap({
      mode: Vim.Mapping.mode,
      maybeKeys: option(string),
    })
  | KeyDisplayer([@opaque] KeyDisplayer.msg);

module Msg = {
  let keybindingsUpdated = keybindings => KeybindingsUpdated(keybindings);
  let vimMap = mapping => VimMap(mapping);
  let vimUnmap = (mode, maybeKeys) => VimUnmap({mode, maybeKeys});
};

// MODEL

type model = {
  userBindings: list(InputStateMachine.uniqueId),
  inputStateMachine: InputStateMachine.t,
  keyDisplayer: option(KeyDisplayer.t),
};

type uniqueId = InputStateMachine.uniqueId;

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
  {inputStateMachine, userBindings: [], keyDisplayer: None};
};

type effect =
  InputStateMachine.effect =
    | Execute(InputStateMachine.command)
    | Text(string)
    | Unhandled(EditorInput.KeyPress.t)
    | RemapRecursionLimitHit;

let keyCodeToString = Sdl2.Keycode.getName;

let keyDown =
    (
      ~config,
      ~key,
      ~context,
      ~time,
      {inputStateMachine, keyDisplayer, _} as model,
    ) => {
  let leaderKey = Configuration.leaderKey.get(config);
  let (inputStateMachine', effects) =
    InputStateMachine.keyDown(~leaderKey, ~key, ~context, inputStateMachine);

  let keyDisplayer' =
    keyDisplayer
    |> Option.map(kd => {
         KeyDisplayer.keyPress(
           ~time=Revery.Time.toFloatSeconds(time),
           keyPressToString(key),
           kd,
         )
       });
  (
    {
      ...model,
      inputStateMachine: inputStateMachine',
      keyDisplayer: keyDisplayer',
    },
    effects,
  );
};

let disable = ({inputStateMachine, _} as model) => {
  ...model,
  inputStateMachine: InputStateMachine.disable(inputStateMachine),
};

let enable = ({inputStateMachine, _} as model) => {
  ...model,
  inputStateMachine: InputStateMachine.enable(inputStateMachine),
};

let text = (~text, ~time, {inputStateMachine, keyDisplayer, _} as model) => {
  let (inputStateMachine', effects) =
    InputStateMachine.text(~text, inputStateMachine);

  let keyDisplayer' =
    keyDisplayer
    |> Option.map(kd => {
         KeyDisplayer.textInput(
           ~time=Revery.Time.toFloatSeconds(time),
           text,
           kd,
         )
       });
  (
    {
      ...model,
      inputStateMachine: inputStateMachine',
      keyDisplayer: keyDisplayer',
    },
    effects,
  );
};

let keyUp = (~config, ~key, ~context, {inputStateMachine, _} as model) => {
  let leaderKey = Configuration.leaderKey.get(config);
  let (inputStateMachine', effects) =
    InputStateMachine.keyUp(~leaderKey, ~key, ~context, inputStateMachine);
  ({...model, inputStateMachine: inputStateMachine'}, effects);
};

let candidates = (~config, ~context, {inputStateMachine, _}) => {
  let leaderKey = Configuration.leaderKey.get(config);
  InputStateMachine.candidates(~leaderKey, ~context, inputStateMachine);
};

let consumedKeys = ({inputStateMachine, _}) =>
  inputStateMachine |> InputStateMachine.consumedKeys;

let addKeyBinding = (~binding, {inputStateMachine, _} as model) => {
  open Schema;
  let (inputStateMachine', uniqueId) =
    InputStateMachine.addBinding(
      binding.matcher,
      binding.condition,
      binding.command,
      inputStateMachine,
    );
  ({...model, inputStateMachine: inputStateMachine'}, uniqueId);
};

let remove = (uniqueId, {inputStateMachine, _} as model) => {
  let inputStateMachine' =
    InputStateMachine.remove(uniqueId, inputStateMachine);
  {...model, inputStateMachine: inputStateMachine'};
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

    {
      ...model,
      inputStateMachine: inputStateMachine'',
      userBindings: userBindingIds,
    };
  };
};

let update = (msg, model) => {
  switch (msg) {
  | Command(ShowDebugInput) => (model, DebugInputShown)
  | Command(DisableKeyDisplayer) => (
      {...model, keyDisplayer: None},
      Nothing,
    )
  | Command(EnableKeyDisplayer) => (
      {...model, keyDisplayer: Some(KeyDisplayer.initial)},
      Nothing,
    )
  | VimMap(mapping) =>
    // When parsing Vim-style mappings, don't require a shift key.
    // In other words - characters like 'J' should resolve to 'Shift+j'
    let explicitShiftKeyNeeded = false;
    let maybeMatcher =
      EditorInput.Matcher.parse(
        ~explicitShiftKeyNeeded,
        ~getKeycode,
        ~getScancode,
        mapping.fromKeys,
      );
    let (model, eff) =
      switch (
        VimCommandParser.parse(~scriptId=mapping.scriptId, mapping.toValue)
      ) {
      | KeySequence(toValue) =>
        let maybeKeys =
          EditorInput.KeyPress.parse(
            ~explicitShiftKeyNeeded,
            ~getKeycode,
            ~getScancode,
            toValue,
          );

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

      | ExCommand(exCmd) =>
        let model' =
          maybeMatcher
          |> Result.map(matcher => {
               let (inputStateMachine', _mappingId) =
                 InputStateMachine.addBinding(
                   matcher,
                   Internal.vimMapModeToWhenExpr(mapping.mode),
                   VimExCommand(exCmd),
                   model.inputStateMachine,
                 );
               {...model, inputStateMachine: inputStateMachine'};
             })
          |> ResultEx.value(~default=model);
        (model', Nothing);
      };
    (model, eff);

  | VimUnmap(_) => (model, Nothing)
  // TODO:
  // | VimUnmap({mode, maybeKeys}) => (model, Nothing)

  | KeybindingsUpdated(bindings) => (
      Internal.updateKeybindings(bindings, model),
      Nothing,
    )

  | KeyDisplayer(msg) =>
    let keyDisplayer' =
      model.keyDisplayer |> Option.map(KeyDisplayer.update(msg));
    ({...model, keyDisplayer: keyDisplayer'}, Nothing);
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

  let disableKeyDisplayer =
    define(
      ~category="Input",
      ~title="Disable Key Displayer",
      ~isEnabledWhen=WhenExpr.parse("keyDisplayerEnabled"),
      "keyDisplayer.disable",
      Command(DisableKeyDisplayer),
    );

  let enableKeyDisplayer =
    define(
      ~category="Input",
      ~title="Enable Key Displayer",
      ~isEnabledWhen=WhenExpr.parse("!keyDisplayerEnabled"),
      "keyDisplayer.enable",
      Command(EnableKeyDisplayer),
    );
};

// SUBSCRIPTION

let sub = ({keyDisplayer, _}) => {
  switch (keyDisplayer) {
  | None => Isolinear.Sub.none
  | Some(kd) =>
    KeyDisplayer.sub(kd) |> Isolinear.Sub.map(msg => KeyDisplayer(msg))
  };
};

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let keyDisplayerEnabled =
    bool("keyDisplayerEnabled", ({keyDisplayer, _}) => keyDisplayer != None);
};

module Contributions = {
  let commands =
    Commands.[showInputState, enableKeyDisplayer, disableKeyDisplayer];

  let configuration = Configuration.[leaderKey.spec];

  let contextKeys = model => {
    WhenExpr.ContextKeys.(
      ContextKeys.[keyDisplayerEnabled]
      |> Schema.fromList
      |> fromSchema(model)
    );
  };
};

// VIEW

module View = {
  open Revery.UI;

  module Overlay = {
    let make = (~input, ~uiFont, ~bottom, ~right, ()) => {
      switch (input.keyDisplayer) {
      | None => React.empty
      | Some(keyDisplayer) =>
        <KeyDisplayer model=keyDisplayer uiFont bottom right />
      };
    };
  };

  module Matcher = {
    open EditorInput;
    open EditorInput.Matcher;
    let make = (~matcher: EditorInput.Matcher.t, ~font: UiFont.t, ()) => {
      switch (matcher) {
      | AllKeysReleased => React.empty
      | Sequence(matchers) =>
        let text =
          matchers
          |> List.map(KeyPress.toString(~keyCodeToString))
          |> String.concat(", ");
        <Text text fontFamily={font.family} fontSize={font.size} />;
      };
    };
  };
};
