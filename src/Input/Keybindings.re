open Types.Input;

module Keybinding = {
  type t = {
    key: string,
    command: string,
    when: Expression.t,
  };

  [@deriving yojson({strict: false, exn: false})]
  type json = {
    key: string,
    command: string,
    when: string,
  };

  let of_yojson = (json: Yojson.Safe.json) => {
    switch (json_of_yjson(json)) {
    | Error(err) => Error(err)
    | Ok(v) =>
      switch (When.parse(v.when)) {
      | Error(err) => Error(err)
      | Ok(v) => Ok(v)
      }
    }
  };
};

[@deriving yojson({strict: false, exn: false})]
type json_keybindings = {bindings: list(Keybinding.json)};

let default = [];

[@deriving (show({with_path: false}), yojson({strict: false, exn: false}))]
type json_keybindings = {bindings: t};

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
