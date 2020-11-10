open Feature_Input.Schema;

module Json = Oni_Core.Json;

module Keybinding = {
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

module Internal = {
  let of_yojson_with_errors:
    list(Yojson.Safe.t) =>
    (list(Feature_Input.Schema.keybinding), list(string)) =
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

// Old version of keybindings - the legacy format:
// { bindings: [ ..bindings. ] }
module Legacy = {
  let upgrade:
    list(Feature_Input.Schema.keybinding) =>
    list(Feature_Input.Schema.keybinding) =
    keys => {
      let upgradeBinding = (binding: Feature_Input.Schema.keybinding) => {
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

let evaluateBindings = (bindings: list(keybinding), errors) => {
  let rec loop = (bindings, errors, currentBindings) => {
    switch (bindings) {
    | [hd, ...tail] =>
      switch (Feature_Input.Schema.resolve(hd)) {
      | Ok(newBinding) =>
        loop(tail, errors, [newBinding, ...currentBindings])
      | Error(msg) => loop(tail, [msg, ...errors], currentBindings)
      }
    | [] => (currentBindings, errors)
    };
  };

  loop(bindings, errors, []);
};

let of_yojson_with_errors = json => {
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
       evaluateBindings(bindings, errors)
     });
};
