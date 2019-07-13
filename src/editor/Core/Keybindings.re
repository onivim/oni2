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

[@deriving (show({with_path: false}), yojson({strict: false, exn: false}))]
type json_keybindings = {bindings: t};

let ofFile = filePath =>
  Yojson.Safe.from_file(filePath) |> json_keybindings_of_yojson;

let getBundledKeybindingsPath = () => {
  Rench.Path.join(
    Utility.executingDirectory,
    "keybindings.json",
  );
};

let get = () => {
  switch (ofFile(getBundledKeybindingsPath())) {
  | Ok(b) => b.bindings
  | Error(e) =>
    Log.error("Error parsing keybindings file ------- " ++ e);
    [];
  };
};
