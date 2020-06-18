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

type windowDisplayMode =
  | Minimized
  | Windowed
  | Maximized
  | Fullscreen;

type t = {
  buffers: Buffers.t,
  bufferRenderers: BufferRenderers.t,
  bufferHighlights: BufferHighlights.t,
  changelog: Feature_Changelog.model,
  colorTheme: Feature_Theme.model,
  commands: Feature_Commands.model(Actions.t),
  contextMenu: Feature_ContextMenu.model,
  completions: Completions.t,
  config: Feature_Configuration.model,
  configuration: Configuration.t,
  decorationProviders: list(DecorationProvider.t),
  diagnostics: Diagnostics.t,
  definition: Definition.t,
  editorFont: Service_Font.font,
  formatting: Feature_Formatting.model,
  terminalFont: Service_Font.font,
  uiFont: UiFont.t,
  quickmenu: option(Quickmenu.t),
  sideBar: SideBar.t,
  // Token theme is theming for syntax highlights
  tokenTheme: TokenTheme.t,
  editorGroups: EditorGroups.t,
  extensions: Extensions.t,
  iconTheme: IconTheme.t,
  isQuitting: bool,
  keyBindings: Keybindings.t,
  keyDisplayer: option(KeyDisplayer.t),
  languageFeatures: LanguageFeatures.t,
  languageInfo: Ext.LanguageInfo.t,
  grammarRepository: Oni_Syntax.GrammarRepository.t,
  lifecycle: Lifecycle.t,
  notifications: Feature_Notification.model,
  references: References.t,
  scm: Feature_SCM.model,
  sneak: Feature_Sneak.model,
  statusBar: Feature_StatusBar.model,
  syntaxHighlights: Feature_Syntax.t,
  terminals: Feature_Terminal.t,
  layout: Feature_Layout.model,
  fileExplorer: FileExplorer.t,
  hover: Feature_Hover.model,
  signatureHelp: Feature_SignatureHelp.model,
  // [windowTitle] is the title of the window
  windowTitle: string,
  windowIsFocused: bool,
  windowDisplayMode,
  workspace: Workspace.t,
  zenMode: bool,
  // State of the bottom pane
  pane: Pane.t,
  searchPane: Feature_Search.model,
  focus: Focus.stack,
  modal: option(Feature_Modals.model),
  textContentProviders: list((int, string)),
  vim: Feature_Vim.model,
};

let initial = (~getUserSettings, ~contributedCommands, ~workingDirectory) => {
  let editorGroups = EditorGroups.create();

  {
    buffers: Buffers.empty,
    bufferHighlights: BufferHighlights.initial,
    bufferRenderers: BufferRenderers.initial,
    changelog: Feature_Changelog.initial,
    colorTheme:
      Feature_Theme.initial([
        Feature_Terminal.Contributions.colors,
        Feature_Notification.Contributions.colors,
      ]),
    commands: Feature_Commands.initial(contributedCommands),
    contextMenu: Feature_ContextMenu.initial,
    completions: Completions.initial,
    config:
      Feature_Configuration.initial(
        ~getUserSettings,
        [
          Feature_Editor.Contributions.configuration,
          Feature_Syntax.Contributions.configuration,
          Feature_Terminal.Contributions.configuration,
        ],
      ),
    configuration: Configuration.default,
    decorationProviders: [],
    definition: Definition.empty,
    diagnostics: Diagnostics.create(),
    quickmenu: None,
    editorFont: Service_Font.default,
    terminalFont: Service_Font.default,
    extensions: Extensions.empty,
    formatting: Feature_Formatting.initial,
    languageFeatures: LanguageFeatures.empty,
    lifecycle: Lifecycle.create(),
    uiFont: UiFont.default,
    sideBar: SideBar.initial,
    tokenTheme: TokenTheme.empty,
    editorGroups,
    iconTheme: IconTheme.create(),
    isQuitting: false,
    keyBindings: Keybindings.empty,
    keyDisplayer: None,
    languageInfo: Ext.LanguageInfo.initial,
    grammarRepository: Oni_Syntax.GrammarRepository.empty,
    notifications: Feature_Notification.initial,
    references: References.initial,
    scm: Feature_SCM.initial,
    sneak: Feature_Sneak.initial,
    statusBar: Feature_StatusBar.initial,
    syntaxHighlights: Feature_Syntax.empty,
    layout: Feature_Layout.initial,
    windowTitle: "",
    windowIsFocused: true,
    windowDisplayMode: Windowed,
    workspace: Workspace.initial(workingDirectory),
    fileExplorer: FileExplorer.initial,
    hover: Feature_Hover.initial,
    signatureHelp: Feature_SignatureHelp.initial,
    zenMode: false,
    pane: Pane.initial,
    searchPane: Feature_Search.initial,
    focus: Focus.initial,
    modal: None,
    terminals: Feature_Terminal.initial,
    textContentProviders: [],
    vim: Feature_Vim.initial,
  };
};

let commands = state =>
  Command.Lookup.unionMany([
    Feature_Commands.all(state.commands),
    Extensions.commands(state.extensions)
    |> Command.Lookup.fromList
    |> Command.Lookup.map(msg => Actions.Extension(msg)),
  ]);

let menus = state => {
  let commands = commands(state);

  Extensions.menus(state.extensions) |> Menu.Lookup.fromSchema(commands);
};
