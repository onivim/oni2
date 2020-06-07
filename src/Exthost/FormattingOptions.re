open Oni_Core;

type t = {
  tabSize: int,
  insertSpaces: bool,
};

let encode = opts =>
  Json.Encode.(
    obj([
      ("tabSize", opts.tabSize |> int),
      ("insertSpaces", opts.insertSpaces |> bool),
    ])
  );
