/*
 * Setup.re
 *
 * Runtime configuration of dependencies
 */

/* open Rench; */

[@deriving yojson({strict: false, exn: true})]
type t = {
  [@key "neovim"]
  neovimPath: string,
  [@key "node"]
  nodePath: string,
};

let ofString = s => {
  Yojson.Safe.from_string(s) |> of_yojson_exn;
};

let ofFile = filePath => {
  Pervasives.open_in(filePath) |> Pervasives.input_line |> ofString;
};

let init = () => {
  Revery_Core.Environment.getExecutingDirectory() ++ "setup.json" |> ofFile;
};
