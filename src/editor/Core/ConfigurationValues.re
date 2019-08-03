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
  editorLargeFileOptimizations: bool,
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
  editorRulers: list(int),
  workbenchActivityBarVisible: bool,
  /* Onivim2 specific setting */
  workbenchSideBarVisible: bool,
  workbenchEditorShowTabs: bool,
  workbenchStatusBarVisible: bool,
  workbenchIconTheme: string,
  filesExclude: list(string),
  zenModeHideTabs: bool,
};

let default = {
  editorDetectIndentation: true,
  editorLargeFileOptimizations: true,
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
  editorRulers: [],
  workbenchActivityBarVisible: true,
  workbenchEditorShowTabs: true,
  workbenchSideBarVisible: true,
  workbenchStatusBarVisible: true,
  workbenchIconTheme: "vs-seti",
  filesExclude: ["node_modules", "_esy"],
  zenModeHideTabs: true,
};
