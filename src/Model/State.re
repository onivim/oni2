/*
 * State.re
 *
 * Top-level state of the editor
 */

open Oni_Core;
open Oni_Input;
open Oni_Syntax;

module Ext = Oni_Extensions;
module ContextMenu = Oni_Components.ContextMenu;
module KeyDisplayer = Oni_Components.KeyDisplayer;
module Completions = Feature_LanguageSupport.Completions;
module Diagnostics = Feature_LanguageSupport.Diagnostics;
module Definition = Feature_LanguageSupport.Definition;
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;
module BufferSyntaxHighlights = Feature_Editor.BufferSyntaxHighlights;

type t = {
  buffers: Buffers.t,
  bufferRenderers: BufferRenderers.t,
  bufferHighlights: BufferHighlights.t,
  bufferSyntaxHighlights: BufferSyntaxHighlights.t,
  commands: Commands.t,
  contextMenu: option(ContextMenu.t(Actions.t)),
  mode: Vim.Mode.t,
  completions: Completions.t,
  configuration: Configuration.t,
  decorationProviders: list(DecorationProvider.t),
  diagnostics: Diagnostics.t,
  definition: Definition.t,
  editorFont: EditorFont.t,
  uiFont: UiFont.t,
  quickmenu: option(Quickmenu.t),
  sideBar: SideBar.t,
  // Theme is the UI shell theming
  theme: Theme.t,
  // Token theme is theming for syntax highlights
  tokenTheme: TokenTheme.t,
  editorGroups: EditorGroups.t,
  extensions: Extensions.t,
  iconTheme: IconTheme.t,
  keyBindings: Keybindings.t,
  keyDisplayer: option(KeyDisplayer.t),
  languageFeatures: LanguageFeatures.t,
  languageInfo: Ext.LanguageInfo.t,
  lifecycle: Lifecycle.t,
  notifications: Notifications.t,
  references: References.t,
  scm: Feature_SCM.model,
  sneak: Sneak.t,
  statusBar: StatusBarModel.t,
  syntaxHighlightingEnabled: bool,
  syntaxClient: option(Oni_Syntax_Client.t),
  terminals: Feature_Terminal.t,
  windowManager: WindowManager.t,
  fileExplorer: FileExplorer.t,
  // [windowTitle] is the title of the window
  windowTitle: string,
  windowIsFocused: bool,
  windowIsMaximized: bool,
  workspace: Workspace.t,
  zenMode: bool,
  // [darkMode] describes if the UI is in 'dark' or 'light' mode.
  // Generally controlled by the theme.
  darkMode: bool,
  // State of the bottom pane
  pane: Pane.t,
  searchPane: Feature_Search.model,
  focus: Focus.stack,
  modal: option(Modal.t(Actions.t)),
  textContentProviders: list((int, string)),
};

let create: unit => t =
  () => {
    buffers: Buffers.empty,
    bufferHighlights: BufferHighlights.initial,
    bufferRenderers: BufferRenderers.initial,
    bufferSyntaxHighlights: BufferSyntaxHighlights.empty,
    commands: Commands.empty,
    contextMenu: None,
    completions: Completions.initial,
    configuration: Configuration.default,
    decorationProviders: [],
    definition: Definition.empty,
    diagnostics: Diagnostics.create(),
    mode: Normal,
    quickmenu: None,
    editorFont:
      EditorFont.create(
        ~fontFile="FiraCode-Regular.ttf",
        ~fontSize=14.,
        ~measuredWidth=1.,
        ~measuredHeight=1.,
        ~descenderHeight=0.,
        (),
      ),
    extensions: Extensions.empty,
    languageFeatures: LanguageFeatures.empty,
    lifecycle: Lifecycle.create(),
    uiFont: UiFont.default,
    sideBar: SideBar.initial,
    theme: Theme.default,
    tokenTheme: TokenTheme.empty,
    editorGroups: EditorGroups.create(),
    iconTheme: IconTheme.create(),
    keyBindings: Keybindings.empty,
    keyDisplayer: None,
    languageInfo: Ext.LanguageInfo.initial,
    notifications: Notifications.initial,
    references: References.initial,
    scm: Feature_SCM.initial,
    sneak: Sneak.initial,
    statusBar: StatusBarModel.create(),
    syntaxHighlightingEnabled: false,
    syntaxClient: None,
    windowManager: WindowManager.create(),
    windowTitle: "",
    windowIsFocused: true,
    windowIsMaximized: false,
    workspace: Workspace.initial,
    fileExplorer: FileExplorer.initial,
    zenMode: false,
    darkMode: true,
    pane: Pane.initial,
    searchPane: Feature_Search.initial,
    focus: Focus.initial,
    modal: None,
    terminals: Feature_Terminal.initial,
    textContentProviders: [],
  };
