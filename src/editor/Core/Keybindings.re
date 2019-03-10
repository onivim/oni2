open Types.Input;

[@deriving (show, yojson({strict: false, exn: false}))]
type keyBindings = {
  key: string,
  command: string,
  condition: controlMode,
};

[@deriving (show, yojson({strict: false, exn: false}))]
type t = list(keyBindings);

[@deriving (show, yojson({strict: false, exn: false}))]
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
