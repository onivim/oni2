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

[@deriving (show({with_path: false}), yojson({strict: false}))]
type t = {
  [@key "editor.lineNumbers"]
  editorLineNumbers: LineNumber.setting,
  [@key "editor.minimap.enabled"]
  editorMinimapEnabled: bool,
  [@key "editor.minimap.showSlider"]
  editorMinimapShowSlider: bool,
  [@key "editor.tablineMode"]
  editorTablineMode,
};

let default = {
  editorMinimapEnabled: true,
  editorMinimapShowSlider: true,
  editorTablineMode: Buffers,
  editorLineNumbers: Relative,
};

let ofFile = filePath => Yojson.Safe.from_file(filePath) |> of_yojson;

let getConfigPath = () => {
  let {configPath, _}: Setup.t = Setup.init();
  configPath;
};

let create = (~configPath=getConfigPath(), ()) =>
  switch (ofFile(configPath)) {
  | Ok(config) => config
  | Error(loc) =>
    print_endline("Error Loc: " ++ loc);
    default;
  };
