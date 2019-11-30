open Oni_Core;

module List = Utility.List;

module Keybinding = {
  type t = {
    key: string,
    command: string,
    condition: Expression.t,
  };

  let condition_of_yojson = (json: Yojson.Safe.t) => {
    switch (json) {
    | `String(v) =>
      switch (When.parse(v)) {
      | Error(err) => Error(err)
      | Ok(condition) => Ok(condition)
      }
    | `Null => Ok(Expression.True)
    | _ => Error("Expected string for condition")
    };
  };

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

let empty = [];

type t = list(Keybinding.t);

let of_yojson_with_errors:
  Yojson.Safe.t => result((list(Keybinding.t), list(string)), string) =
  json => {
    let bindingsJson =
      switch (json) {
      // Current format:
      // [ ...bindings ]
      | `List(bindingsJson) => Ok(bindingsJson)
      // Legacy format:
      // { bindings: [ ..bindings. ] }
      | `Assoc([("bindings", `List(bindingsJson))]) => Ok(bindingsJson)
      | _ => Error("Unable to parse keybindings - not a JSON array.")
      };

    switch (bindingsJson) {
    | Error(msg) => Error(msg)
    | Ok(bindings) =>
      let parsedBindings =
        bindings
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

      Ok((bindings, errors));
    };
  };
