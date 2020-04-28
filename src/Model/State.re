/*
 * State.re
 *
 * Top-level state of the editor
 */

open Oni_Core;
open Oni_Input;
open Oni_Syntax;

module Ext = Oni_Extensions;
module KeyDisplayer = Oni_Components.KeyDisplayer;
module Completions = Feature_LanguageSupport.Completions;
module Diagnostics = Feature_LanguageSupport.Diagnostics;
module Definition = Feature_LanguageSupport.Definition;
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;

module ContextMenu = {
  type t =
    | NotificationStatusBarItem
    | Nothing;
};

type t = {
  buffers: Buffers.t,
  bufferRenderers: BufferRenderers.t,
  bufferHighlights: BufferHighlights.t,
  colorTheme: Feature_Theme.model,
  commands: Feature_Commands.model(Actions.t),
  contextMenu: ContextMenu.t,
  vimMode: Vim.Mode.t,
  completions: Completions.t,
  config: Feature_Configuration.model,
  configuration: Configuration.t,
  decorationProviders: list(DecorationProvider.t),
  diagnostics: Diagnostics.t,
  definition: Definition.t,
  editorFont: Service_Font.font,
  terminalFont: Service_Font.font,
  uiFont: UiFont.t,
  quickmenu: option(Quickmenu.t),
  sideBar: SideBar.t,
  // Token theme is theming for syntax highlights
  tokenTheme: TokenTheme.t,
  editorGroups: EditorGroups.t,
  extensions: Extensions.t,
  iconTheme: Exthost.Types.IconTheme.t,
  isQuitting: bool,
  keyBindings: Keybindings.t,
  keyDisplayer: option(KeyDisplayer.t),
  languageFeatures: LanguageFeatures.t,
  languageInfo: Ext.LanguageInfo.t,
  lifecycle: Lifecycle.t,
  notifications: Feature_Notification.model,
  references: References.t,
  scm: Feature_SCM.model,
  sneak: Sneak.t,
  statusBar: StatusBarModel.t,
  syntaxClient: option(Oni_Syntax_Client.t),
  syntaxHighlights: Feature_Syntax.t,
  terminals: Feature_Terminal.t,
  layout: Feature_Layout.t(int),
  fileExplorer: FileExplorer.t,
  // [windowTitle] is the title of the window
  windowTitle: string,
  windowIsFocused: bool,
  windowIsMaximized: bool,
  workspace: Workspace.t,
  zenMode: bool,
  // State of the bottom pane
  pane: Pane.t,
  searchPane: Feature_Search.model,
  focus: Focus.stack,
  modal: option(Feature_Modals.model),
  textContentProviders: list((int, string)),
};

let initial = (~getUserSettings, ~contributedCommands) => {
  buffers: Buffers.empty,
  bufferHighlights: BufferHighlights.initial,
  bufferRenderers: BufferRenderers.initial,
  colorTheme:
    Feature_Theme.initial([
      Feature_Terminal.Contributions.colors,
      Feature_Notification.Contributions.colors,
    ]),
  commands: Feature_Commands.initial(contributedCommands),
  contextMenu: ContextMenu.Nothing,
  completions: Completions.initial,
  config:
    Feature_Configuration.initial(
      ~getUserSettings,
      [Feature_Editor.Contributions.configuration],
    ),
  configuration: Configuration.default,
  decorationProviders: [],
  definition: Definition.empty,
  diagnostics: Diagnostics.create(),
  vimMode: Normal,
  quickmenu: None,
  editorFont: Service_Font.default,
  terminalFont: Service_Font.default,
  extensions: Extensions.empty,
  languageFeatures: LanguageFeatures.empty,
  lifecycle: Lifecycle.create(),
  uiFont: UiFont.default,
  sideBar: SideBar.initial,
  tokenTheme: TokenTheme.empty,
  editorGroups: EditorGroups.create(),
  iconTheme: Exthost.Types.IconTheme.create(),
  isQuitting: false,
  keyBindings: Keybindings.empty,
  keyDisplayer: None,
  languageInfo: Ext.LanguageInfo.initial,
  notifications: Feature_Notification.initial,
  references: References.initial,
  scm: Feature_SCM.initial,
  sneak: Sneak.initial,
  statusBar: StatusBarModel.create(),
  syntaxClient: None,
  syntaxHighlights: Feature_Syntax.empty,
  layout: Feature_Layout.initial,
  windowTitle: "",
  windowIsFocused: true,
  windowIsMaximized: false,
  workspace: Workspace.initial,
  fileExplorer: FileExplorer.initial,
  zenMode: false,
  pane: Pane.initial,
  searchPane: Feature_Search.initial,
  focus: Focus.initial,
  modal: None,
  terminals: Feature_Terminal.initial,
  textContentProviders: [],
};

let commands = state =>
  Exthost.Types.Command.Lookup.unionMany([
    Feature_Commands.all(state.commands),
    Extensions.commands(state.extensions)
    |> Exthost.Types.Command.Lookup.fromList
    |> Exthost.Types.Command.Lookup.map(msg => Actions.Extension(msg)),
  ]);

let menus = state => {
  let commands = commands(state);

  Extensions.menus(state.extensions)
  |> Exthost.Types.Menu.Lookup.fromSchema(commands);
};
