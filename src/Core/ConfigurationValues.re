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

type autoReveal = [ | `HighlightAndScroll | `HighlightOnly | `NoReveal];

type t = {
  editorAutoClosingBrackets: autoClosingBrackets,
  explorerAutoReveal: autoReveal,
  workbenchActivityBarVisible: bool,
  workbenchColorTheme: string,
  workbenchIconTheme: string,
  workbenchEditorShowTabs: bool,
  workbenchEditorEnablePreview: bool,
  workbenchStatusBarVisible: bool,
  filesExclude: list(string),
  vsync: Revery.Vsync.t,
  vimUseSystemClipboard,
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
  explorerAutoReveal: `HighlightAndScroll,
  workbenchActivityBarVisible: true,
  workbenchColorTheme: Constants.defaultTheme,
  workbenchEditorShowTabs: true,
  workbenchEditorEnablePreview: true,
  workbenchStatusBarVisible: true,
  workbenchIconTheme: "vs-seti",
  filesExclude: ["_esy", "node_modules", ".git"],
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
