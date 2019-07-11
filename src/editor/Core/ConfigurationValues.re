/*
 * ConfigurationValues.re
 *
 * Configuration settings for the editor
 */

[@deriving show({with_path: false})]
type editorRenderWhitespace =
  | All
  | Boundary
  | None;

[@deriving show({with_path: false})]
type t = {
  editorDetectIndentation: bool,
  editorLineNumbers: LineNumber.setting,
  editorMatchBrackets: bool,
  editorMinimapEnabled: bool,
  editorMinimapShowSlider: bool,
  editorMinimapMaxColumn: int,
  editorInsertSpaces: bool,
  editorIndentSize: int,
  editorTabSize: int,
  editorHighlightActiveIndentGuide: bool,
  editorRenderIndentGuides: bool,
  editorRenderWhitespace,
  workbenchEditorShowTabs: bool,
  workbenchIconTheme: string,
  filesExclude: list(string),
};

let default = {
  editorDetectIndentation: true,
  editorMatchBrackets: true,
  editorMinimapEnabled: true,
  editorMinimapShowSlider: true,
  editorMinimapMaxColumn: Constants.default.minimapMaxColumn,
  editorLineNumbers: On,
  editorInsertSpaces: false,
  editorIndentSize: 4,
  editorTabSize: 4,
  editorRenderIndentGuides: true,
  editorHighlightActiveIndentGuide: true,
  editorRenderWhitespace: All,
  workbenchEditorShowTabs: true,
  workbenchIconTheme: "vs-seti",
  filesExclude: ["node_modules", "_esy"],
};

let getBundledConfigPath = () => {
  Rench.Path.join(
    Rench.Environment.getExecutingDirectory(),
    "configuration.json",
  );
};
