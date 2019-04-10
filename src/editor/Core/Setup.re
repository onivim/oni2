/*
 * Setup.re
 *
 * Runtime configuration of dependencies
 */

[@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
type t = {
  [@key "neovim"]
  neovimPath: string,
  [@key "node"]
  nodePath: string,
  [@key "textmateService"]
  textmateServicePath: string,
  [@key "bundledExtensions"]
  bundledExtensionsPath: string,
  [@key "developmentExtensions"]
  developmentExtensionsPath: [@default None] option(string),
  [@key "extensionHost"]
  extensionHostPath: string,
  [@key "keybindings"]
  keybindingsPath: string,
  [@key "rg"]
  rgPath: string,
  version: [@default "Unknown"] string,
};

let ofString = str => Yojson.Safe.from_string(str) |> of_yojson_exn;

let ofFile = filePath => Yojson.Safe.from_file(filePath) |> of_yojson_exn;

let init = () =>
  Revery.Environment.getExecutingDirectory() ++ "setup.json" |> ofFile;
