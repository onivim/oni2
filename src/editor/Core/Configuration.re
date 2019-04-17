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

[@deriving (show({with_path: false}), yojson)]
type editorRenderWhitespace =
  | [@name "all"] All
  | [@name "boundary"] Boundary
  | [@name "None"] None;

[@deriving (show({with_path: false}), yojson({strict: false}))]
type t = {
  [@key "editor.lineNumbers"]
  editorLineNumbers: LineNumber.setting,
  [@key "editor.minimap.enabled"]
  editorMinimapEnabled: bool,
  [@key "editor.minimap.showSlider"]
  editorMinimapShowSlider: bool,
  [@key "editor.minimap.maxColumn"]
  editorMinimapMaxColumn: int,
  [@key "editor.tablineMode"]
  editorTablineMode,
  [@key "editor.insertSpaces"]
  editorInsertSpaces: bool,
  [@key "editor.indentSize"]
  editorIndentSize: int,
  [@key "editor.tabSize"]
  editorTabSize: int,
  [@key "editor.highlightActiveIndentGuide"]
  editorHighlightActiveIndentGuide: bool,
  [@key "editor.renderIndentGuides"]
  editorRenderIndentGuides: bool,
  [@key "editor.renderWhitespace"]
  editorRenderWhitespace,
  [@key "workbench.iconTheme"]
  workbenchIconTheme: string,
};

let default = {
  editorMinimapEnabled: true,
  editorMinimapShowSlider: true,
  editorMinimapMaxColumn: 12,
  editorTablineMode: Buffers,
  editorLineNumbers: Relative,
  editorInsertSpaces: false,
  editorIndentSize: 4,
  editorTabSize: 4,
  editorRenderIndentGuides: true,
  editorHighlightActiveIndentGuide: true,
  editorRenderWhitespace: All,
  workbenchIconTheme: "vs-seti",
};

let ofFile = filePath => Yojson.Safe.from_file(filePath) |> of_yojson;

let getBundledConfigPath = () => {
  Rench.Path.join(
    Rench.Environment.getExecutingDirectory(),
    "configuration.json",
  );
};

let create = (~configPath=getBundledConfigPath(), ()) =>
  switch (ofFile(configPath)) {
  | Ok(config) => config
  | Error(loc) =>
    prerr_endline("Error Loc: " ++ loc);
    default;
  };
