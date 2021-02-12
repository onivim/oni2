open Feature_Input.Schema;

module Json = Oni_Core.Json;

module Keybinding = {
  module Decode = {
    open Json.Decode;

    let andExpressions =
      list(
        one_of([
          ("string", string |> map(str => WhenExpr.Defined(str))),
          ("fallback", succeed(WhenExpr.Value(False))),
        ]),
      )
      |> map(items => WhenExpr.And(items));

    let condition =
      one_of([
        ("list", list(andExpressions) |> map(items => WhenExpr.Or(items))),
        ("string", string |> map(WhenExpr.parse)),
        ("null", null |> map(_ => WhenExpr.Value(True))),
      ]);

    let binding =
      obj(({field, _}) => {
        let arguments =
          field.optional("args", value) |> Option.value(~default=`Null);
        Feature_Input.Schema.bindWithArgs(
          ~arguments,
          ~key=field.required("key", string),
          ~command=field.required("command", string),
          ~condition=
            field.withDefault("when", WhenExpr.Value(True), condition),
        );
      });
  };

  let of_yojson = (json: Yojson.Safe.t) => {
    json
    |> Json.Decode.decode_value(Decode.binding)
    |> Result.map_error(Json.Decode.string_of_error);
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
        let f = cmd =>
          switch (cmd) {
          | "quickOpen.open" => "workbench.action.quickOpen"
          | "commandPalette.open" => "workbench.action.showCommands"
          | str => str
          };

        Feature_Input.Schema.mapCommand(~f, binding);
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
