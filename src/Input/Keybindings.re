type keybinding = {
  key: string,
  command: string,
  condition: WhenExpr.t,
};

module Json = Oni_Core.Json;

module Keybinding = {
  type t = keybinding;

  let parseSimple =
    fun
    | `String(s) => WhenExpr.Defined(s)
    | _ => WhenExpr.Value(False);

  let parseAndExpression =
    fun
    | `String(expr) => WhenExpr.Defined(expr)
    | `List(andExpressions) =>
      WhenExpr.And(List.map(parseSimple, andExpressions))
    | _ => WhenExpr.Value(False);

  let condition_of_yojson =
    fun
    | `List(orExpressions) =>
      Ok(WhenExpr.Or(List.map(parseAndExpression, orExpressions)))
    | `String(v) => Ok(WhenExpr.parse(v))
    | `Null => Ok(WhenExpr.Value(True))
    | _ => Error("Expected string for condition");

  let of_yojson = (json: Yojson.Safe.t) => {
    let key = Yojson.Safe.Util.member("key", json);
    let command = Yojson.Safe.Util.member("command", json);
    let condition =
      Yojson.Safe.Util.member("when", json) |> condition_of_yojson;

    let wrapError = msg => Error(msg ++ ": " ++ Yojson.Safe.to_string(json));

    switch (condition) {
    | Ok(condition) =>
      switch (key, command) {
      | (`String(key), `String(command)) => Ok({key, command, condition})
      | (`String(_), _) => wrapError("'command' is required")
      | (_, `String(_)) => wrapError("'key' is required")
      | _ => wrapError("'key' and 'command' are required")
      }

    | Error(_) as err => err
    };
  };
};
let keyToSdlName =
  EditorInput.Key.(
    {
      fun
      | Escape => "Escape"
      | Return => "Return"
      | Up => "Up"
      | Down => "Down"
      | Left => "Left"
      | Right => "Right"
      | Tab => "Tab"
      | PageUp => "PageUp"
      | PageDown => "PageDown"
      | Delete => "Delete"
      | Pause => "Pause"
      | Home => "Home"
      | End => "End"
      | Backspace => "Backspace"
      | CapsLock => "CapsLock"
      | Insert => "Insert"
      | Function(digit) => "F" ++ string_of_int(digit)
      | Space => "Space"
      | NumpadDigit(digit) => "Keypad " ++ string_of_int(digit)
      | NumpadMultiply => "Keypad *"
      | NumpadAdd => "Keypad +"
      | NumpadSeparator => "Keypad "
      | NumpadSubtract => "Keypad -"
      | NumpadDivide => "Keypad //"
      | NumpadDecimal => "Keypad ."
      | Character(c) => String.make(1, c) |> String.uppercase_ascii;
    }
  );


module Input =
  EditorInput.Make({
    type context = WhenExpr.ContextKeys.t;
    type command = string;

let getKeycode = inputKey => {
  inputKey
  |> keyToSdlName
  |> Sdl2.Keycode.ofName
  |> (
    fun
    | 0 => None
    | x => Some(x)
  );
};

let getScancode = inputKey => {
  inputKey
  |> keyToSdlName
  |> Sdl2.Scancode.ofName
  |> (
    fun
    | 0 => None
    | x => Some(x)
  );
};
  });

module Internal = {
  let of_yojson_with_errors:
    list(Yojson.Safe.t) => (list(Keybinding.t), list(string)) =
    bindingsJson => {
      let parsedBindings =
        bindingsJson
        // Parse each binding
        |> List.map(Keybinding.of_yojson);

      // Get errors from individual keybindings, but don't let them stop parsing
      let errors =
        List.filter_map(
          keyBinding =>
            switch (keyBinding) {
            | Ok(_) => None
            | Error(msg) => Some(msg)
            },
          parsedBindings,
        );

      // Get valid bindings now
      let bindings =
        List.filter_map(
          keyBinding =>
            switch (keyBinding) {
            | Ok(v) => Some(v)
            | Error(_) => None
            },
          parsedBindings,
        );

      (bindings, errors);
    };
};

type effect =
  Input.effect =
    | Execute(string) | Text(string) | Unhandled(EditorInput.KeyPress.t);

type t = Input.t;

let empty = Input.empty;
let count = Input.count;
let keyDown = Input.keyDown;
let text = Input.text;
let keyUp = Input.keyUp;

// Old version of keybindings - the legacy format:
// { bindings: [ ..bindings. ] }
module Legacy = {
  let upgrade: list(Keybinding.t) => list(Keybinding.t) =
    keys => {
      let upgradeBinding = (binding: Keybinding.t) => {
        switch (binding.command) {
        | "quickOpen.open" => {
            ...binding,
            command: "workbench.action.quickOpen",
          }
        | "commandPalette.open" => {
            ...binding,
            command: "workbench.action.showCommands",
          }
        | _ => binding
        };
      };

      keys |> List.map(upgradeBinding);
    };
};

let addBinding = ({key, command, condition}, bindings) => {
  let evaluateCondition = (whenExpr, contextKeys) => {
    WhenExpr.evaluate(whenExpr, WhenExpr.ContextKeys.getValue(contextKeys));
  };

  let matchers = Input.Matcher.parse(key);

  matchers
  |> Stdlib.Result.map(m => {
       let (bindings, _id) =
         Input.addBinding(
           m,
           evaluateCondition(condition),
           command,
           bindings,
         );
       bindings;
     });
};

let evaluateBindings = (bindings: list(keybinding), errors) => {
  let rec loop = (bindings, errors, currentBindings) => {
    switch (bindings) {
    | [hd, ...tail] =>
      switch (addBinding(hd, currentBindings)) {
      | Ok(newBindings) => loop(tail, errors, newBindings)
      | Error(msg) => loop(tail, [msg, ...errors], currentBindings)
      }
    | [] => (currentBindings, errors)
    };
  };

  loop(bindings, errors, Input.empty);
};

let of_yojson_with_errors = (~default=[], json) => {
  let previous =
    switch (json) {
    // Current format:
    // [ ...bindings ]
    | `List(bindingsJson) =>
      let res = bindingsJson |> Internal.of_yojson_with_errors;
      Ok(res);

    // Legacy format:
    // { bindings: [ ..bindings. ] }
    | `Assoc([("bindings", `List(bindingsJson))]) =>
      let res = bindingsJson |> Internal.of_yojson_with_errors;

      Ok(res)
      |> Stdlib.Result.map(((bindings, errors)) => {
           (Legacy.upgrade(bindings), errors)
         });

    | _ => Error("Unable to parse keybindings - not a JSON array.")
    };

  previous
  |> Stdlib.Result.map(((bindings, errors)) => {
       let combinedBindings = default @ bindings;
       evaluateBindings(combinedBindings, errors);
     });
};
