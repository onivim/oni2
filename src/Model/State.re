/*
 * State.re
 *
 * Top-level state of the editor
 */

open Oni_Core;
open Oni_Input;
open Oni_Syntax;

module Ext = Oni_Extensions;

type t = {
  buffers: Buffers.t,
  bufferRenderers: BufferRenderers.t,
  bufferHighlights: BufferHighlights.t,
  bufferSyntaxHighlights: BufferSyntaxHighlights.t,
  commands: Commands.t,
  mode: Vim.Mode.t,
  completions: Completions.t,
  diagnostics: Diagnostics.t,
  definition: Definition.t,
  editorFont: EditorFont.t,
  uiFont: UiFont.t,
  hover: Hover.t,
  quickmenu: option(Quickmenu.t),
  configuration: Configuration.t,
  sideBar: SideBar.t,
  // Theme is the UI shell theming
  theme: Theme.t,
  // Token theme is theming for syntax highlights
  tokenTheme: TokenTheme.t,
  editorGroups: EditorGroups.t,
  extensions: Extensions.t,
  iconTheme: IconTheme.t,
  keyBindings: Keybindings.t,
  keyDisplayer: KeyDisplayer.t,
  languageFeatures: LanguageFeatures.t,
  languageInfo: Ext.LanguageInfo.t,
  lifecycle: Lifecycle.t,
  notifications: Notifications.t,
  references: References.t,
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
  focus: Focus.stack,
};

let create: unit => t =
  () => {
    buffers: Buffers.empty,
    bufferHighlights: BufferHighlights.initial,
    bufferRenderers: BufferRenderers.initial,
    bufferSyntaxHighlights: BufferSyntaxHighlights.empty,
    commands: Commands.empty,
    completions: Completions.default,
    configuration: Configuration.default,
    definition: Definition.empty,
    diagnostics: Diagnostics.create(),
    hover: Hover.empty,
    mode: Normal,
    quickmenu: None,
    editorFont:
      EditorFont.create(
        ~fontFile="FiraCode-Regular.ttf",
        ~boldFontFile="FiraCode-Bold.ttf",
        ~boldItalicFontFile="FiraCode-Bold.ttf",
        ~italicFontFile="FiraCode-Regular.ttf",
        ~fontSize=14,
        ~measuredWidth=1.,
        ~measuredHeight=1.,
        (),
      ),
    extensions: Extensions.empty,
    languageFeatures: LanguageFeatures.empty,
    lifecycle: Lifecycle.create(),
    uiFont: UiFont.create(~fontFile="selawk.ttf", ~fontSize=12, ()),
    sideBar: SideBar.initial,
    theme: Theme.default,
    tokenTheme: TokenTheme.empty,
    editorGroups: EditorGroups.create(),
    iconTheme: IconTheme.create(),
    keyBindings: Keybindings.empty,
    keyDisplayer: KeyDisplayer.empty,
    languageInfo: Ext.LanguageInfo.initial,
    notifications: Notifications.default,
    references: References.initial,
    statusBar: StatusBarModel.create(),
    windowManager: WindowManager.create(),
    windowTitle: "",
    workspace: Workspace.initial,
    fileExplorer: FileExplorer.initial,
    zenMode: false,
    darkMode: true,
    searchPane: None,
    focus: Focus.initial,
  };
