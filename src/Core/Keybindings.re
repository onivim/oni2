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

let getDefaultKeybinds = () => {
  switch (ConfigurationDefaults.getDefaultConfigString("keybindings.json")) {
  | Some(c) => Yojson.Safe.from_string(c) |> json_keybindings_of_yojson
  | None => Error("Unable to generate keybinding config")
  };
};

let getKeybinds = () => {
  let configPath = Filesystem.getOrCreateConfigFile("keybindings.json");
  switch (configPath) {
  | Ok(keybindPathAsString) =>
    switch (Yojson.Safe.from_file(keybindPathAsString)) {
    | v => json_keybindings_of_yojson(v)
    | exception (Yojson.Json_error(msg)) =>
      Log.error("Error loading keybindings file: " ++ msg);
      getDefaultKeybinds();
    }
  | Error(err) =>
    Log.error("Error loading keybindings file: " ++ err);
    getDefaultKeybinds();
  };
};

let get = () => {
  switch (getKeybinds()) {
  | Ok(b) => b.bindings
  | Error(e) =>
    Log.error("Error parsing keybindings file ------- " ++ e);
    [];
  };
};
