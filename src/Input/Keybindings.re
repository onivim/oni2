open Oni_Core;
open Oni_Core.Types.Input;

module Keybinding = {
  type t = {
    key: string,
    command: string,
    condition: Expression.t,
  };

  module Json = {
    [@deriving yojson({strict: false, exn: false})]
    type t = {
      key: string,
      command: string,
      [@key "when"]
      condition: string,
    };
  };

  let of_yojson = (json: Yojson.Safe.t) => {
    switch (Json.of_yojson(json)) {
    | Error(err) => Error(err)
    | Ok(v) =>
      switch (Expression.parse(v.condition)) {
      | Error(err) => Error(err)
      | Ok(condition) => Ok({key: v.key, command: v.command, condition})
      }
    };
  };

  let to_yojson = (v: t) => {
    failwith("Not implemented");
  };
};

[@deriving yojson({strict: false, exn: false})]
type json_keybindings = {bindings: list(Keybinding.t)};

let default = [];

type t = list(Keybinding.t);

let getDefaultConfig = () => {
  switch (ConfigurationDefaults.getDefaultConfigString("keybindings.json")) {
  | Some(c) => Yojson.Safe.from_string(c) |> json_keybindings_of_yojson
  | None => Error("Unable to generate config")
  };
};

let get = () => {
  switch (getDefaultConfig()) {
  | Ok(b) => b.bindings
  | Error(e) =>
    Log.error("Error parsing keybindings file ------- " ++ e);
    [];
  };
};
