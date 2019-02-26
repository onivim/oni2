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
  [@key "editor.lineNumbers"]
  editorLineNumbers: LineNumber.setting,
  [@key "editor.minimapEnabled"]
  editorMinimapEnabled: bool,
  [@key "editor.tablineMode"]
  editorTablineMode,
};

let ofFile = filePath => Yojson.Safe.from_file(filePath) |> of_yojson_exn;

let getConfigPath = () => {
  let {configPath, _}: Setup.t = Setup.init();
  configPath;
};

let create = (~configPath=getConfigPath(), ()) => {
  let config = ofFile(configPath);

  print_endline(show(config));
  config;
};
