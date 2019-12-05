/*
 * State.re
 *
 * Top-level state of the editor
 */

open Oni_Core;
open Oni_Core.Types;
open Oni_Input;
open Oni_Syntax;

module Ext = Oni_Extensions;

type t = {
  buffers: Buffers.t,
  bufferSyntaxHighlights: BufferSyntaxHighlights.t,
  commands: Commands.t,
  mode: Vim.Mode.t,
  completions: Completions.t,
  diagnostics: Diagnostics.t,
  editorFont: EditorFont.t,
  uiFont: UiFont.t,
  hover: Hover.t,
  quickmenu: option(Quickmenu.t),
  configuration: Configuration.t,
  // New-school native syntax highlighting
  // TODO: Remove
  syntaxHighlighting: SyntaxHighlighting.t,
  // Theme is the UI shell theming
  theme: Theme.t,
  // Token theme is theming for syntax highlights
  tokenTheme: TokenTheme.t,
  editorGroups: EditorGroups.t,
  extensions: Extensions.t,
  iconTheme: IconTheme.t,
  keyBindings: Keybindings.t,
  keyDisplayer: KeyDisplayer.t,
  languageFeatures: Ext.LanguageFeatures.t,
  languageInfo: Ext.LanguageInfo.t,
  lifecycle: Lifecycle.t,
  notifications: Notifications.t,
  searchHighlights: SearchHighlights.t,
  statusBar: StatusBarModel.t,
  windowManager: WindowManager.t,
  fileExplorer: FileExplorer.t,
  // [windowTitle] is the title of the window
  windowTitle: string,
  workspace: Workspace.t,
  zenMode: bool,
  // [darkMode] describes if the UI is in 'dark' or 'light' mode.
  // Generally controlled by the theme.
  darkMode: bool,
  searchPane: option(Search.t),
};

let create: unit => t =
  () => {
    buffers: Buffers.empty,
    bufferSyntaxHighlights: BufferSyntaxHighlights.empty,
    commands: Commands.empty,
    completions: Completions.default,
    configuration: Configuration.default,
    diagnostics: Diagnostics.create(),
    hover: Hover.empty,
    mode: Normal,
    quickmenu: None,
    editorFont:
      EditorFont.create(
        ~fontFile="FiraCode-Regular.ttf",
        ~fontSize=14,
        ~measuredWidth=1.,
        ~measuredHeight=1.,
        (),
      ),
    extensions: Extensions.empty,
    languageFeatures: Ext.LanguageFeatures.empty,
    lifecycle: Lifecycle.create(),
    uiFont: UiFont.create(~fontFile="selawk.ttf", ~fontSize=12, ()),
    // TODO: Remove
    syntaxHighlighting: SyntaxHighlighting.empty,
    theme: Theme.default,
    tokenTheme: TokenTheme.empty,
    editorGroups: EditorGroups.create(),
    iconTheme: IconTheme.create(),
    keyBindings: Keybindings.empty,
    keyDisplayer: KeyDisplayer.empty,
    languageInfo: Ext.LanguageInfo.empty,
    notifications: Notifications.default,
    searchHighlights: SearchHighlights.create(),
    statusBar: StatusBarModel.create(),
    windowManager: WindowManager.create(),
    windowTitle: "",
    workspace: Workspace.initial,
    fileExplorer: FileExplorer.initial,
    zenMode: false,
    darkMode: true,
    searchPane: None,
  };
