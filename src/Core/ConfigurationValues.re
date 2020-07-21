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
type autoClosingBrackets =
  | Never
  | LanguageDefined;

[@deriving show({with_path: false})]
type fontSmoothing =
  | Default
  | None
  | Antialiased
  | SubpixelAntialiased;

type quickSuggestionsEnabled = {
  other: bool,
  comments: bool,
  strings: bool,
};

type t = {
  editorAutoClosingBrackets: autoClosingBrackets,
  editorDetectIndentation: bool,
  editorFontFile: string,
  editorFontSize: float,
  editorFontLigatures: bool,
  editorFontSmoothing: fontSmoothing,
  editorHoverDelay: int,
  editorHoverEnabled: bool,
  editorLargeFileOptimizations: bool,
  editorLineNumbers: LineNumber.setting,
  editorMatchBrackets: bool,
  editorAcceptSuggestionOnEnter: [ | `on | `off | `smart],
  editorMinimapEnabled: bool,
  editorMinimapShowSlider: bool,
  editorMinimapMaxColumn: int,
  editorInsertSpaces: bool,
  editorIndentSize: int,
  editorQuickSuggestions: quickSuggestionsEnabled,
  editorTabSize: int,
  editorHighlightActiveIndentGuide: bool,
  editorRenderIndentGuides: bool,
  editorRenderWhitespace,
  editorRulers: list(int),
  terminalIntegratedFontFile: string,
  terminalIntegratedFontSize: float,
  terminalIntegratedFontSmoothing: fontSmoothing,
  workbenchActivityBarVisible: bool,
  workbenchColorTheme: string,
  workbenchIconTheme: string,
  /* Onivim2 specific setting */
  workbenchSideBarVisible: bool,
  workbenchEditorShowTabs: bool,
  workbenchStatusBarVisible: bool,
  workbenchTreeIndent: int,
  filesExclude: list(string),
  vsync: Revery.Vsync.t,
  vimUseSystemClipboard,
  uiShadows: bool,
  uiZoom: float,
  windowTitle: string,
  zenModeHideTabs: bool,
  zenModeSingleFile: bool,
  // Experimental feature flags
  // These are 'use-at-your-own-risk' features
  // Turn on tree-sitter for supported filetypes:
  // - JSON
  experimentalVimL: list(string),
};

let default = {
  editorAutoClosingBrackets: LanguageDefined,
  editorDetectIndentation: true,
  editorFontFile: Constants.defaultFontFile,
  editorFontSmoothing: Default,
  editorFontSize: Constants.defaultFontSize,
  editorFontLigatures: true,
  editorHoverDelay: 1000,
  editorHoverEnabled: true,
  editorLargeFileOptimizations: true,
  editorMatchBrackets: true,
  editorAcceptSuggestionOnEnter: `on,
  editorMinimapEnabled: true,
  editorMinimapShowSlider: true,
  editorMinimapMaxColumn: Constants.minimapMaxColumn,
  editorLineNumbers: On,
  editorInsertSpaces: true,
  editorIndentSize: 4,
  editorTabSize: 4,
  editorRenderIndentGuides: true,
  editorHighlightActiveIndentGuide: true,
  editorQuickSuggestions: {
    other: true,
    comments: true,
    strings: true,
  },
  editorRenderWhitespace: None,
  editorRulers: [],
  terminalIntegratedFontFile: Constants.defaultFontFile,
  terminalIntegratedFontSize: Constants.defaultTerminalFontSize,
  terminalIntegratedFontSmoothing: Default,
  workbenchActivityBarVisible: true,
  workbenchColorTheme: "LaserWave Italic",
  workbenchEditorShowTabs: true,
  workbenchSideBarVisible: true,
  workbenchStatusBarVisible: true,
  workbenchIconTheme: "vs-seti",
  workbenchTreeIndent: 2,
  filesExclude: ["_esy", "node_modules", ".git"],
  uiShadows: true,
  uiZoom: 1.0,
  vimUseSystemClipboard: {
    yank: true,
    delete: false,
    paste: false,
  },
  vsync: Revery.Vsync.Immediate,
  windowTitle: "${dirty}${activeEditorShort}${separator}${rootName}${separator}${appName}",
  zenModeHideTabs: true,
  zenModeSingleFile: true,
  experimentalVimL: [],
};
