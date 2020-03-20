type effect =
  | Command(string)
  | Unhandled(EditorInput.key);

// TODO;
let count = _ => 0;

type keybinding = {
  key: string,
  command: string,
  condition: WhenExpr.t,
};

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

    switch (condition) {
    | Ok(condition) =>
      switch (key, command) {
      | (`String(key), `String(command)) => Ok({key, command, condition})
      | (`String(_), _) => Error("Command must be a string")
      | (_, `String(_)) => Error("Key must be a string")
      | _ => Error("Binding must specify key and command strings")
      }

    | Error(msg) => Error(msg)
    };
  };
};

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

let empty = [];

type t = list(Keybinding.t);

let _keyToVimString = (key: EditorInput.key) => {
  let name = ref(Sdl2.Keycode.getName(key.keycode));

  if (key.modifiers.alt) {
    name := "A-" ++ name^;
  };

  if (key.modifiers.control) {
    name := "C-" ++ name^;
  };

  if (key.modifiers.shift) {
    name := "S-" ++ name^;
  };

  if (key.modifiers.meta) {
    name := "D-" ++ name^;
  };

  let wrapIfLong = str =>
    if (String.length(str) > 1) {
      "<" ++ str ++ ">";
    } else {
      str;
    };

  let convertSdlName =
    fun
    | "<ESCAPE>" => "<ESC>"
    | "<RETURN>" => "<CR>"
    | v => v;

  name^ |> String.uppercase_ascii |> wrapIfLong |> convertSdlName;
};

let keyDown = (~context, ~key, bindings) => {
  let keyStr = _keyToVimString(key);

  prerr_endline("KEYSTR: " ++ keyStr);

  let getValue = propertyName =>
    switch (Hashtbl.find_opt(context, propertyName)) {
    | Some(true) => WhenExpr.Value.True
    | Some(false)
    | None => WhenExpr.Value.False
    };

  let effects =
    List.fold_left(
      (defaultAction, {key, command, condition}) =>
        Handler.matchesCondition(condition, keyStr, key, getValue)
          ? [Command(command)] : defaultAction,
      [],
      bindings,
    );

  if (effects == []) {
    (bindings, [Unhandled(key)]);
  } else {
    (bindings, effects);
  };
};

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
       (default @ bindings, errors)
     });
};
