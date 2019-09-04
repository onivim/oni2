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
type vimUseSystemClipboard = {
  yank: bool,
  delete: bool,
  paste: bool,
};

[@deriving show({with_path: false})]
type t = {
  editorDetectIndentation: bool,
  editorFontFamily: string,
  editorFontSize: int,
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
  workbenchTreeIndent: int,
  filesExclude: list(string),
  vimUseSystemClipboard,
  uiShadows: bool,
  zenModeHideTabs: bool,
  zenModeSingleFile: bool,
  // Experimental feature flags
  // These are 'use-at-your-own-risk' features
  experimentalTreeSitter: bool,
};

let default = {
  editorDetectIndentation: true,
  editorFontFamily: "FiraCode-Regular.ttf",
  editorFontSize: 14,
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
  workbenchTreeIndent: 2,
  filesExclude: ["node_modules", "_esy"],
  uiShadows: true,
  vimUseSystemClipboard: {
    yank: true,
    delete: false,
    paste: false,
  },
  zenModeHideTabs: true,
  zenModeSingleFile: true,
  experimentalTreeSitter: false,
};
