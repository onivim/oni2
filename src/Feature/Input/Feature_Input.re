open Oni_Core;
open Utility;
module Log = (val Log.withNamespace("Oni2.Feature.Input"));

module KeybindingsLoader = KeybindingsLoader;
module Schema = Schema;

// TODO: Move to Service_Input
module ReveryKeyConverter = ReveryKeyConverter;

let keyPressToString = key => {
  key |> EditorInput.KeyPress.toString(~keyToString=EditorInput.Key.toString);
};

let keyCandidateToString = keyCandidate => {
  keyCandidate
  |> EditorInput.KeyCandidate.toList
  // TODO: Alternate strategy - maybe choose shortest option from list?
  |> (
    list =>
      List.nth_opt(list, 0)
      |> Option.map(keyPressToString)
      |> Option.value(~default="?Unknown key?")
  );
};

type timeout =
  | NoTimeout
  | Timeout(Revery.Time.t);

// VIM SETTINGS

module VimSettings = {
  open Config.Schema;
  open VimSetting.Schema;

  let timeout =
    vim2("timeout", "timeoutlen", (maybeTimeout, maybeTimeoutLen) => {
      let maybeTimeoutBool =
        maybeTimeout |> OptionEx.flatMap(VimSetting.decode_value_opt(bool));
      let maybeTimeoutLenInt =
        maybeTimeoutLen |> OptionEx.flatMap(VimSetting.decode_value_opt(int));
      switch (maybeTimeoutBool, maybeTimeoutLenInt) {
      | (None, Some(timeoutLength))
      | (Some(true), Some(timeoutLength)) =>
        Some(Timeout(Revery.Time.milliseconds(timeoutLength)))

      | (Some(true), None) => Some(Timeout(Revery.Time.seconds(1)))

      | (Some(false), _) => Some(NoTimeout)

      | (None, None) => None
      };
    });
};

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
                  //~keyCodeToString=Sdl2.Keycode.getName,
                  EditorInput.KeyPress.PhysicalKey(key),
                )
                |> string
              | None => "(none)" |> string
              };
            }
          ),
      );

    module Timeout = {
      let decode =
        Json.Decode.(
          one_of([
            (
              "bool",
              bool
              |> map(
                   fun
                   | true => Timeout(Revery.Time.seconds(1))
                   | false => NoTimeout,
                 ),
            ),
            (
              "int",
              int
              |> map(
                   fun
                   | 0 => NoTimeout
                   | milliseconds =>
                     Timeout(Revery.Time.milliseconds(milliseconds)),
                 ),
            ),
          ])
        );

      let encode =
        Json.Encode.(
          fun
          | NoTimeout => bool(false)
          | Timeout(time) =>
            int(
              time
              |> Revery.Time.toFloatSeconds
              |> (t => t *. 1000.)
              |> int_of_float,
            )
        );
    };

    let timeout = custom(~decode=Timeout.decode, ~encode=Timeout.encode);
  };

  let leaderKey =
    setting("vim.leader", CustomDecoders.physicalKey, ~default=None);

  let timeout =
    setting(
      ~vim=VimSettings.timeout,
      "vim.timeout",
      CustomDecoders.timeout,
      ~default=Timeout(Revery.Time.seconds(1)),
    );
};

// MSG

type outmsg =
  | Nothing
  | DebugInputShown
  | ErrorNotifications(list(string))
  | MapParseError({
      fromKeys: string,
      toKeys: string,
      error: string,
    })
  | OpenFile(FpExp.t(FpExp.absolute))
  | TimedOut;

type execute =
  InputStateMachine.execute =
    | NamedCommand({
        command: string,
        arguments: Yojson.Safe.t,
      })
    | VimExCommand(string);

[@deriving show]
type command =
  | ShowDebugInput
  | EnableKeyDisplayer
  | DisableKeyDisplayer
  | OpenKeybindingsFile;

[@deriving show]
type msg =
  | Command(command)
  | KeybindingsUpdated([@opaque] list(Schema.resolvedKeybinding))
  | KeybindingsReloaded({
      bindings: [@opaque] list(Schema.resolvedKeybinding),
      errors: list(string),
    })
  | VimMap(Vim.Mapping.t)
  | VimUnmap({
      mode: Vim.Mapping.mode,
      maybeKeys: option(string),
    })
  | KeyDisplayer([@opaque] KeyDisplayer.msg)
  | Timeout;

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
  // Keep track of the input tick - an incrementing number on every input event -
  // such that we can provide a unique id for the timer to flush on timeout.
  inputTick: int,
  keybindingLoader: KeybindingsLoader.t,
};

type uniqueId = InputStateMachine.uniqueId;

let incrementTick = ({inputTick, _} as model) => {
  ...model,
  inputTick: inputTick + 1,
};

let initial = (~loader, keybindings) => {
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

           | Ok(ResolvedBinding({matcher, condition, command, _})) =>
             let (ism, _bindingId) =
               InputStateMachine.addBinding(matcher, condition, command, ism);
             ism;

           | Ok(
               ResolvedRemap({allowRecursive, matcher, condition, toKeys, _}),
             ) =>
             let (ism, _bindingId) =
               InputStateMachine.addMapping(
                 ~allowRecursive,
                 matcher,
                 condition,
                 toKeys,
                 ism,
               );
             ism;
           }
         },
         InputStateMachine.empty,
       );
  {
    inputStateMachine,
    userBindings: [],
    keyDisplayer: None,
    inputTick: 0,
    keybindingLoader: loader,
  };
};

type effect =
  InputStateMachine.effect =
    | Execute(InputStateMachine.command)
    | Text(string)
    | Unhandled({
        key: EditorInput.KeyCandidate.t,
        isProducedByRemap: bool,
      })
    | RemapRecursionLimitHit;

let keyDown =
    (
      ~config,
      ~scancode,
      ~key,
      ~context,
      ~time,
      {inputStateMachine, keyDisplayer, _} as model,
    ) => {
  let leaderKey = Configuration.leaderKey.get(config);

  let (inputStateMachine', effects) =
    InputStateMachine.keyDown(
      ~leaderKey,
      ~scancode,
      ~key,
      ~context,
      inputStateMachine,
    );

  let keyDisplayer' =
    keyDisplayer
    |> Option.map(kd => {
         KeyDisplayer.keyPress(
           ~time=Revery.Time.toFloatSeconds(time),
           keyCandidateToString(key),
           kd,
         )
       });
  (
    {
      ...model,
      inputStateMachine: inputStateMachine',
      keyDisplayer: keyDisplayer',
    }
    |> incrementTick,
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

let notifyFileSaved = (path, {keybindingLoader, _} as model) => {
  ...model,
  keybindingLoader: KeybindingsLoader.notifyFileSaved(path, keybindingLoader),
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
    }
    |> incrementTick,
    effects,
  );
};

let keyUp = (~config, ~scancode, ~context, {inputStateMachine, _} as model) => {
  let leaderKey = Configuration.leaderKey.get(config);
  let (inputStateMachine', effects) =
    InputStateMachine.keyUp(
      ~leaderKey,
      ~scancode,
      ~context,
      inputStateMachine,
    );
  (
    {...model, inputStateMachine: inputStateMachine'} |> incrementTick,
    effects,
  );
};

let timeout = (~context, {inputStateMachine, _} as model) => {
  let (inputStateMachine', effects) =
    InputStateMachine.timeout(~context, inputStateMachine);
  ({...model, inputStateMachine: inputStateMachine'}, effects);
};

let candidates = (~config, ~context, {inputStateMachine, _}) => {
  let leaderKey = Configuration.leaderKey.get(config);
  InputStateMachine.candidates(~leaderKey, ~context, inputStateMachine);
};

let commandToAvailableBindings = (~command, ~config, ~context, model) => {
  let allCandidates = candidates(~config, ~context, model);

  if (String.length(command) <= 0) {
    [];
  } else {
    allCandidates
    |> List.filter_map(((matcher: EditorInput.Matcher.t, ex: execute)) =>
         switch (ex) {
         | NamedCommand({command: namedCommand, _})
             when command == namedCommand =>
           switch (matcher) {
           | Sequence(keys) => Some(keys)
           | AllKeysReleased => None
           }
         | _ => None
         }
       );
  };
};

let consumedKeys = ({inputStateMachine, _}) =>
  inputStateMachine |> InputStateMachine.consumedKeys;

let addKeyBinding = (~binding, {inputStateMachine, _} as model) => {
  Schema.(
    switch (binding) {
    | ResolvedBinding(binding) =>
      let (inputStateMachine', uniqueId) =
        InputStateMachine.addBinding(
          binding.matcher,
          binding.condition,
          binding.command,
          inputStateMachine,
        );
      ({...model, inputStateMachine: inputStateMachine'}, uniqueId);

    | ResolvedRemap(remap) =>
      let (inputStateMachine', uniqueId) =
        InputStateMachine.addMapping(
          ~allowRecursive=remap.allowRecursive,
          remap.matcher,
          remap.condition,
          remap.toKeys,
          inputStateMachine,
        );
      ({...model, inputStateMachine: inputStateMachine'}, uniqueId);
    }
  );
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
             | NormalAndVisualAndSelectAndOperator =>
               "selectMode || normalMode || visualMode || operatorPending"
               |> parse;
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
               switch (resolvedBinding) {
               | ResolvedBinding({matcher, condition, command, _}) =>
                 InputStateMachine.addBinding(
                   matcher,
                   condition,
                   command,
                   ism,
                 )

               | ResolvedRemap({
                   allowRecursive,
                   matcher,
                   condition,
                   toKeys,
                   _,
                 }) =>
                 InputStateMachine.addMapping(
                   ~allowRecursive,
                   matcher,
                   condition,
                   toKeys,
                   ism,
                 )
               };
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
  | Command(OpenKeybindingsFile) =>
    let eff =
      model.keybindingLoader
      |> KeybindingsLoader.getFilePath
      |> Option.map(path => OpenFile(path))
      |> Option.value(~default=Nothing);
    (model, eff);
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
      EditorInput.Matcher.parse(~explicitShiftKeyNeeded, mapping.fromKeys);
    let (model, eff) =
      switch (
        VimCommandParser.parse(~scriptId=mapping.scriptId, mapping.toValue)
      ) {
      | KeySequence(toValue) =>
        let maybeKeys =
          EditorInput.KeyPress.parse(~explicitShiftKeyNeeded, toValue);

        let maybeModel =
          ResultEx.map2(
            (matcher, keys) => {
              let (inputStateMachine', _mappingId) =
                InputStateMachine.addMapping(
                  ~allowRecursive=mapping.recursive,
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

  | KeybindingsReloaded({bindings, errors}) =>
    let outmsg =
      switch (errors) {
      | [] => Nothing
      | errors => ErrorNotifications(errors)
      };
    (Internal.updateKeybindings(bindings, model), outmsg);

  | Timeout => (model, TimedOut)

  | KeyDisplayer(msg) =>
    let keyDisplayer' =
      model.keyDisplayer |> Option.map(KeyDisplayer.update(msg));
    ({...model, keyDisplayer: keyDisplayer'}, Nothing);
  };
};

// COMMANDS

module Commands = {
  open Feature_Commands.Schema;

  let openDefaultKeybindingsFile =
    define(
      ~category="Preferences",
      ~title="Open keybindings file",
      "workbench.action.openDefaultKeybindingsFile",
      Command(OpenKeybindingsFile),
    );

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

let sub =
    (
      ~config,
      {keyDisplayer, inputTick, inputStateMachine, keybindingLoader, _},
    ) => {
  let keyDisplayerSub =
    switch (keyDisplayer) {
    | None => Isolinear.Sub.none
    | Some(kd) =>
      KeyDisplayer.sub(kd) |> Isolinear.Sub.map(msg => KeyDisplayer(msg))
    };

  let timeoutSub =
    switch (Configuration.timeout.get(config)) {
    | NoTimeout => Isolinear.Sub.none
    | Timeout(delay) =>
      if (InputStateMachine.isPending(inputStateMachine)) {
        Service_Time.Sub.once(
          ~uniqueId="Feature_Input.keyExpirer:" ++ string_of_int(inputTick),
          ~delay,
          ~msg=(~current as _) => {
          Timeout
        });
      } else {
        Isolinear.Sub.none;
      }
    };

  let loaderSub =
    KeybindingsLoader.sub(keybindingLoader)
    |> Isolinear.Sub.map(((bindings, errors)) => {
         KeybindingsReloaded({bindings, errors})
       });

  [keyDisplayerSub, timeoutSub, loaderSub] |> Isolinear.Sub.batch;
};

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let keyDisplayerEnabled =
    bool("keyDisplayerEnabled", ({keyDisplayer, _}) => keyDisplayer != None);
};

module Contributions = {
  let commands =
    Commands.[
      showInputState,
      enableKeyDisplayer,
      disableKeyDisplayer,
      openDefaultKeybindingsFile,
    ];

  let configuration = Configuration.[leaderKey.spec, timeout.spec];

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
          matchers |> List.map(KeyPress.toString) |> String.concat(", ");
        <Text text fontFamily={font.family} fontSize={font.size} />;
      };
    };
  };
};
