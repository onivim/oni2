/*
 * ConfigurationValues.re
 *
 * Configuration settings for the editor
 */

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

type fontLigatures = [ | `Bool(bool) | `List(list(string))];

type autoReveal = [ | `HighlightAndScroll | `HighlightOnly | `NoReveal];

type t = {
  editorAutoClosingBrackets: autoClosingBrackets,
  editorDetectIndentation: bool,
  editorFontLigatures: fontLigatures,
  editorFontSmoothing: fontSmoothing,
  editorHoverDelay: int,
  editorHoverEnabled: bool,
  editorLargeFileOptimizations: bool,
  editorInsertSpaces: bool,
  editorIndentSize: int,
  editorTabSize: int,
  editorHighlightActiveIndentGuide: bool,
  editorRenderIndentGuides: bool,
  editorRulers: list(int),
  explorerAutoReveal: autoReveal,
  terminalIntegratedFontFile: string,
  terminalIntegratedFontSize: float,
  terminalIntegratedFontSmoothing: fontSmoothing,
  workbenchActivityBarVisible: bool,
  workbenchColorTheme: string,
  workbenchIconTheme: string,
  workbenchSideBarLocation: string,
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
  editorFontSmoothing: Default,
  editorFontLigatures: `Bool(true),
  editorHoverDelay: 1000,
  editorHoverEnabled: true,
  editorLargeFileOptimizations: true,
  editorInsertSpaces: true,
  editorIndentSize: 4,
  editorTabSize: 4,
  editorRenderIndentGuides: true,
  editorHighlightActiveIndentGuide: true,
  editorRulers: [],
  explorerAutoReveal: `HighlightAndScroll,
  terminalIntegratedFontFile: Constants.defaultFontFile,
  terminalIntegratedFontSize: Constants.defaultTerminalFontSize,
  terminalIntegratedFontSmoothing: Default,
  workbenchActivityBarVisible: true,
  workbenchColorTheme: "LaserWave Italic",
  workbenchEditorShowTabs: true,
  workbenchSideBarLocation: "left",
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
  zenModeHideTabs: true,
  zenModeSingleFile: true,
  experimentalVimL: [],
};
