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

let get = () => {
  let {keybindingsPath, _}: Setup.t = Setup.init();
  switch (ofFile(keybindingsPath)) {
  | Ok(b) => b.bindings
  | Error(e) =>
    print_endline("Error parsing keybindings file ------- " ++ e);
    [];
  };
};
