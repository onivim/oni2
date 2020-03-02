module Keybinding = {
  type t = {
    key: string,
    command: string,
    condition: WhenExpr.t,
  };

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

let of_yojson_with_errors = json => {
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
};
