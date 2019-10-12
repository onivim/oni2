open Types.Input;

[@deriving (show({with_path: false}), yojson({strict: false, exn: false}))]
type keyBindings = {
  key: string,
  command: string,
  [@key "when"]
  condition: list(controlMode),
};

[@deriving (show({with_path: false}), yojson({strict: false, exn: false}))]
type t = list(keyBindings);

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
