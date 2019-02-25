/*
 * Configuration.re
 *
 * Configuration settings for the editor
 */

[@deriving (show({with_path: false}), yojson)]
type editorTablineMode =
  | [@name "buffers"] Buffers
  | [@name "tabs"] Tabs
  | [@name "hybrid"] Hybrid;

[@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
type t = {
  editorLineNumbers: LineNumber.setting,
  editorMinimapEnabled: bool,
  editorTablineMode,
};

let ofFile = filePath => Yojson.Safe.from_file(filePath) |> of_yojson_exn;

let default = {
  editorTablineMode: Buffers,
  editorMinimapEnabled: true,
  editorLineNumbers: Relative,
};

let getConfigPath = () => {
  let sep = Filename.dir_sep;
  Revery_Core.Environment.getWorkingDirectory()
  ++ sep
  ++ "assets"
  ++ sep
  ++ "configuration"
  ++ sep
  ++ "configuration.json";
};

let create = (~configPath=getConfigPath(), ()) => {
  let config = ofFile(configPath);

  print_endline(show(config));
  config;
};
