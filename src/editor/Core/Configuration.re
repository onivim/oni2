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

let create = () => {
  let config =
    Revery_Core.Environment.getWorkingDirectory()
    ++ "/assets/configuration/configuration.json"
    |> ofFile;

  print_endline(show(config));
  config;
};
