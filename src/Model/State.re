/*
 * State.re
 *
 * Top-level state of the editor
 */

open Oni_Core;
open Oni_Input;
open Oni_Syntax;

module KeyDisplayer = Oni_Components.KeyDisplayer;
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;

type windowDisplayMode =
  | Minimized
  | Windowed
  | Maximized
  | Fullscreen;

type t = {
  buffers: Feature_Buffers.model,
  bufferRenderers: BufferRenderers.t,
  bufferHighlights: BufferHighlights.t,
  changelog: Feature_Changelog.model,
  clipboard: Feature_Clipboard.model,
  colorTheme: Feature_Theme.model,
  commands: Feature_Commands.model(Actions.t),
  contextMenu: Feature_ContextMenu.model,
  config: Feature_Configuration.model,
  configuration: Configuration.t,
  decorations: Feature_Decorations.model,
  diagnostics: Feature_Diagnostics.model,
  editorFont: Service_Font.font,
  input: Feature_Input.model,
  messages: Feature_Messages.model,
  terminalFont: Service_Font.font,
  uiFont: UiFont.t,
  quickmenu: option(Quickmenu.t),
  sideBar: Feature_SideBar.model,
  // Token theme is theming for syntax highlights
  tokenTheme: TokenTheme.t,
  extensions: Feature_Extensions.model,
  exthost: Feature_Exthost.model,
  iconTheme: IconTheme.t,
  isQuitting: bool,
  keyBindings: Keybindings.t,
  keyDisplayer: option(KeyDisplayer.t),
  languageFeatures: LanguageFeatures.t,
  languageSupport: Feature_LanguageSupport.model,
  languageInfo: Exthost.LanguageInfo.t,
  grammarRepository: Oni_Syntax.GrammarRepository.t,
  lifecycle: Lifecycle.t,
  notifications: Feature_Notification.model,
  //  references: References.t,
  registers: Feature_Registers.model,
  scm: Feature_SCM.model,
  sneak: Feature_Sneak.model,
  statusBar: Feature_StatusBar.model,
  syntaxHighlights: Feature_Syntax.t,
  terminals: Feature_Terminal.t,
  layout: Feature_Layout.model,
  fileExplorer: Feature_Explorer.model,
  signatureHelp: Feature_SignatureHelp.model,
  windowIsFocused: bool,
  windowDisplayMode,
  workspace: Workspace.t,
  zenMode: bool,
  // State of the bottom pane
  pane: Feature_Pane.model,
  searchPane: Feature_Search.model,
  focus: Focus.stack,
  modal: option(Feature_Modals.model),
  textContentProviders: list((int, string)),
  vim: Feature_Vim.model,
  autoUpdate: Feature_AutoUpdate.model,
};

let initial =
    (
      ~initialBuffer,
      ~initialBufferRenderers,
      ~extensionGlobalPersistence,
      ~extensionWorkspacePersistence,
      ~getUserSettings,
      ~contributedCommands,
      ~workingDirectory,
      ~extensionsFolder,
    ) => {
  let config =
    Feature_Configuration.initial(
      ~getUserSettings,
      [
        Feature_AutoUpdate.Contributions.configuration,
        Feature_Editor.Contributions.configuration,
        Feature_Syntax.Contributions.configuration,
        Feature_Terminal.Contributions.configuration,
        Feature_LanguageSupport.Contributions.configuration,
        Feature_Layout.Contributions.configuration,
        Feature_TitleBar.Contributions.configuration,
      ],
    );
  let initialEditor = {
    open Feature_Editor;
    let editorBuffer = initialBuffer |> EditorBuffer.ofBuffer;
    let config = Feature_Configuration.resolver(config, Feature_Vim.initial);
    Editor.create(~config, ~buffer=editorBuffer, ());
  };

  {
    buffers: Feature_Buffers.empty |> Feature_Buffers.add(initialBuffer),
    bufferHighlights: BufferHighlights.initial,
    bufferRenderers: initialBufferRenderers,
    changelog: Feature_Changelog.initial,
    clipboard: Feature_Clipboard.initial,
    colorTheme:
      Feature_Theme.initial([
        Feature_LanguageSupport.Contributions.colors,
        Feature_Terminal.Contributions.colors,
        Feature_Notification.Contributions.colors,
      ]),
    commands: Feature_Commands.initial(contributedCommands),
    contextMenu: Feature_ContextMenu.initial,
    config,
    configuration: Configuration.default,
    decorations: Feature_Decorations.initial,
    diagnostics: Feature_Diagnostics.initial,
    input: Feature_Input.initial,
    quickmenu: None,
    editorFont: Service_Font.default,
    terminalFont: Service_Font.default,
    extensions:
      Feature_Extensions.initial(
        ~globalPersistence=extensionGlobalPersistence,
        ~workspacePersistence=extensionWorkspacePersistence,
        ~extensionsFolder,
      ),
    exthost: Feature_Exthost.initial,
    languageFeatures: LanguageFeatures.empty,
    languageSupport: Feature_LanguageSupport.initial,
    lifecycle: Lifecycle.create(),
    messages: Feature_Messages.initial,
    uiFont: UiFont.default,
    sideBar: Feature_SideBar.initial,
    tokenTheme: TokenTheme.empty,
    iconTheme: IconTheme.create(),
    isQuitting: false,
    keyBindings: Keybindings.empty,
    keyDisplayer: None,
    languageInfo: Exthost.LanguageInfo.initial,
    grammarRepository: Oni_Syntax.GrammarRepository.empty,
    notifications: Feature_Notification.initial,
    //    references: References.initial,
    registers: Feature_Registers.initial,
    scm: Feature_SCM.initial,
    sneak: Feature_Sneak.initial,
    statusBar: Feature_StatusBar.initial,
    syntaxHighlights: Feature_Syntax.empty,
    layout: Feature_Layout.initial([initialEditor]),
    windowIsFocused: true,
    windowDisplayMode: Windowed,
    workspace: Workspace.initial(workingDirectory),
    fileExplorer: Feature_Explorer.initial(~rootPath=workingDirectory),
    signatureHelp: Feature_SignatureHelp.initial,
    zenMode: false,
    pane: Feature_Pane.initial,
    searchPane: Feature_Search.initial,
    focus: Focus.initial,
    modal: None,
    terminals: Feature_Terminal.initial,
    textContentProviders: [],
    vim: Feature_Vim.initial,
    autoUpdate: Feature_AutoUpdate.initial,
  };
};
