/*
 * State.re
 *
 * Top-level state of the editor
 */

open Oni_Core;
open Oni_Syntax;

module Commands = GlobalCommands;
let windowCommandCondition = "!insertMode || terminalFocus" |> WhenExpr.parse;

let isMacCondition = "isMac" |> WhenExpr.parse;
let defaultKeyBindings =
  Feature_Input.Schema.[
    {
      key: "<UP>",
      command: Commands.List.focusUp.id,
      condition: "listFocus || textInputFocus" |> WhenExpr.parse,
    },
    {
      key: "<DOWN>",
      command: Commands.List.focusDown.id,
      condition: "listFocus || textInputFocus" |> WhenExpr.parse,
    },
    {
      key: "<RIGHT>",
      command: Commands.List.selectBackground.id,
      condition: "quickmenuCursorEnd" |> WhenExpr.parse,
    },
  ]
  @ Feature_SideBar.Contributions.keybindings
  @ Feature_Input.Schema.[
      {
        key: "<C-TAB>",
        command:
          Commands.Workbench.Action.openNextRecentlyUsedEditorInGroup.id,
        condition: "editorTextFocus || terminalFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-P>",
        command: Commands.Workbench.Action.quickOpen.id,
        condition: "editorTextFocus || terminalFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-P>",
        command: Commands.Workbench.Action.quickOpen.id,
        condition: "isMac" |> WhenExpr.parse,
      },
      {
        key: "<S-C-P>",
        command: Commands.Workbench.Action.showCommands.id,
        condition: "!isMac" |> WhenExpr.parse,
      },
      {
        key: "<D-S-P>",
        command: Commands.Workbench.Action.showCommands.id,
        condition: "isMac" |> WhenExpr.parse,
      },
      {
        key: "<C-V>",
        command: Feature_Clipboard.Commands.paste.id,
        condition:
          "insertMode || textInputFocus || commandLineFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-V>",
        command: Feature_Clipboard.Commands.paste.id,
        condition: isMacCondition,
      },
      {
        key: "<ESC>",
        command: Commands.Workbench.Action.closeQuickOpen.id,
        condition: "inQuickOpen" |> WhenExpr.parse,
      },
      {
        key: "<C-N>",
        command: Commands.List.focusDown.id,
        condition: "listFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-P>",
        command: Commands.List.focusUp.id,
        condition: "listFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-N>",
        command: Commands.List.focusDown.id,
        condition: "isMac && listFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-P>",
        command: Commands.List.focusUp.id,
        condition: "isMac && listFocus" |> WhenExpr.parse,
      },
      {
        key: "<TAB>",
        command: Commands.List.focusDown.id,
        condition: "listFocus" |> WhenExpr.parse,
      },
      {
        key: "<S-TAB>",
        command: Commands.List.focusUp.id,
        condition: "listFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-TAB>",
        command:
          Commands.Workbench.Action.quickOpenNavigateNextInEditorPicker.id,
        condition: "inEditorsPicker" |> WhenExpr.parse,
      },
      {
        key: "<S-C-TAB>",
        command:
          Commands.Workbench.Action.quickOpenNavigatePreviousInEditorPicker.id,
        condition: "inEditorsPicker" |> WhenExpr.parse,
      },
      {
        key: "<release>",
        command: Commands.List.select.id,
        condition: "inEditorsPicker" |> WhenExpr.parse,
      },
    ]
  @ Feature_Registers.Contributions.keybindings
  @ Feature_LanguageSupport.Contributions.keybindings
  @ Feature_Input.Schema.[
      {
        key: "<CR>",
        command: Commands.List.select.id,
        condition: "listFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-Z>",
        command: Commands.undo.id,
        condition: "isMac && editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-S-Z>",
        command: Commands.redo.id,
        condition: "isMac && editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-S>",
        command: Commands.Workbench.Action.Files.save.id,
        condition: "isMac && editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-S>",
        command: Commands.Workbench.Action.Files.save.id,
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-]>",
        command: Commands.Editor.Action.indentLines.id,
        condition: "visualMode" |> WhenExpr.parse,
      },
      {
        key: "<C-[>",
        command: Commands.Editor.Action.outdentLines.id,
        condition: "visualMode" |> WhenExpr.parse,
      },
      {
        key: "<D-]>",
        command: Commands.Editor.Action.indentLines.id,
        condition: "isMac && visualMode" |> WhenExpr.parse,
      },
      {
        key: "<D-[>",
        command: Commands.Editor.Action.outdentLines.id,
        condition: "isMac && visualMode" |> WhenExpr.parse,
      },
      {
        key: "<TAB>",
        command: Commands.indent.id,
        condition: "visualMode" |> WhenExpr.parse,
      },
      {
        key: "<S-TAB>",
        command: Commands.outdent.id,
        condition: "visualMode" |> WhenExpr.parse,
      },
      {
        key: "<C-G>",
        command: Feature_Sneak.Commands.start.id,
        condition: WhenExpr.Value(True),
      },
      {
        key: "<ESC>",
        command: Feature_Sneak.Commands.stop.id,
        condition: "sneakMode" |> WhenExpr.parse,
      },
    ]
  @ Feature_Pane.Contributions.keybindings
  @ Feature_Input.Schema.[
      {
        key: "<D-W>",
        command: Feature_Layout.Commands.closeActiveEditor.id,
        condition: isMacCondition,
      },
      {
        key: "<C-PAGEDOWN>",
        command: Feature_Layout.Commands.nextEditor.id,
        condition: WhenExpr.Value(True),
      },
      {
        key: "<D-S-]>",
        command: Feature_Layout.Commands.nextEditor.id,
        condition: isMacCondition,
      },
      {
        key: "<C-PAGEUP>",
        command: Feature_Layout.Commands.previousEditor.id,
        condition: WhenExpr.Value(True),
      },
      {
        key: "<D-S-[>",
        command: Feature_Layout.Commands.previousEditor.id,
        condition: isMacCondition,
      },
      {
        key: "<D-=>",
        command: "workbench.action.zoomIn",
        condition: isMacCondition,
      },
      {
        key: "<C-=>",
        command: "workbench.action.zoomIn",
        condition: WhenExpr.Value(True),
      },
      {
        key: "<D-->",
        command: "workbench.action.zoomOut",
        condition: isMacCondition,
      },
      {
        key: "<C-->",
        command: "workbench.action.zoomOut",
        condition: WhenExpr.Value(True),
      },
      {
        key: "<D-0>",
        command: "workbench.action.zoomReset",
        condition: isMacCondition,
      },
      {
        key: "<C-0>",
        command: "workbench.action.zoomReset",
        condition: WhenExpr.Value(True),
      },
    ]
  @ Feature_Terminal.Contributions.keybindings
  //LAYOUT
  @ Feature_Input.Schema.[
      {
        key: "<C-W>H",
        command: Feature_Layout.Commands.moveLeft.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W><C-H>",
        command: Feature_Layout.Commands.moveLeft.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W><LEFT>",
        command: Feature_Layout.Commands.moveLeft.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W>L",
        command: Feature_Layout.Commands.moveRight.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W><C-L>",
        command: Feature_Layout.Commands.moveRight.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W><C-S>",
        command: Feature_Layout.Commands.splitHorizontal.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W>S",
        command: Feature_Layout.Commands.splitHorizontal.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W><C-V>",
        command: Feature_Layout.Commands.splitVertical.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W>V",
        command: Feature_Layout.Commands.splitVertical.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W><RIGHT>",
        command: Feature_Layout.Commands.moveRight.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W>K",
        command: Feature_Layout.Commands.moveUp.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W><C-K>",
        command: Feature_Layout.Commands.moveUp.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W><UP>",
        command: Feature_Layout.Commands.moveUp.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W>J",
        command: Feature_Layout.Commands.moveDown.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W><C-J>",
        command: Feature_Layout.Commands.moveDown.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W><DOWN>",
        command: Feature_Layout.Commands.moveDown.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W>R",
        command: Feature_Layout.Commands.rotateForward.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W><C-R>",
        command: Feature_Layout.Commands.rotateForward.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W><C-S-R>",
        command: Feature_Layout.Commands.rotateBackward.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W>-",
        command: Feature_Layout.Commands.decreaseVerticalSize.id,
        condition: windowCommandCondition,
      },
      //      TODO: Does not work, blocked by bug in editor-input
      //      {
      //        key: "<C-W>+",
      //        command: Feature_Layout.Commands.increaseVerticalSize.id,
      //        condition: "!insertMode" |> WhenExpr.parse
      //      },
      {
        key: "<C-W><S-,>", // TODO: Does not work and should be `<`, but blocked by bugs in editor-input,
        command: Feature_Layout.Commands.increaseHorizontalSize.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W><S-.>", // TODO: Does not work and should be `>`, but blocked by bugs in editor-input
        command: Feature_Layout.Commands.decreaseHorizontalSize.id,
        condition: windowCommandCondition,
      },
      {
        key: "<C-W>=",
        command: Feature_Layout.Commands.resetSizes.id,
        condition: windowCommandCondition,
      },
      // TODO: Fails to parse
      // {
      //   key: "<C-W>_",
      //   command: Feature_Layout.Commands.maximizeVertical.id,
      //   condition: windowCommandCondition,
      // },
      // TODO: Fails to parse
      // {
      //   key: "<C-W>|",
      //   command: Feature_Layout.Commands.maximizeHorizontal.id,
      //   condition: windowCommandCondition,
      // },
      {
        key: "<C-W>o",
        command: Feature_Layout.Commands.toggleMaximize.id,
        condition: windowCommandCondition,
      },
      {
        key: "<A-DOWN>",
        command: Feature_SignatureHelp.Commands.incrementSignature.id,
        condition:
          "editorTextFocus && parameterHintsVisible" |> WhenExpr.parse,
      },
      {
        key: "<A-UP>",
        command: Feature_SignatureHelp.Commands.decrementSignature.id,
        condition:
          "editorTextFocus && parameterHintsVisible" |> WhenExpr.parse,
      },
    ]
  @ Component_VimWindows.Contributions.keybindings
  @ Component_VimList.Contributions.keybindings
  @ Component_VimTree.Contributions.keybindings;

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
  cli: Oni_CLI.t,
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
  logging: Feature_Logging.model,
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
  languageSupport: Feature_LanguageSupport.model,
  languageInfo: Exthost.LanguageInfo.t,
  grammarRepository: Oni_Syntax.GrammarRepository.t,
  lifecycle: Lifecycle.t,
  notifications: Feature_Notification.model,
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
  registration: Feature_Registration.model,
};

let initial =
    (
      ~cli,
      ~initialBuffer,
      ~initialBufferRenderers,
      ~extensionGlobalPersistence,
      ~extensionWorkspacePersistence,
      ~getUserSettings,
      ~contributedCommands,
      ~workingDirectory,
      ~extensionsFolder,
      ~licenseKeyPersistence,
    ) => {
  let config =
    Feature_Configuration.initial(
      ~getUserSettings,
      [
        Feature_AutoUpdate.Contributions.configuration,
        Feature_Buffers.Contributions.configuration,
        Feature_Editor.Contributions.configuration,
        Feature_Input.Contributions.configuration,
        Feature_SideBar.Contributions.configuration,
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
    let config =
      Feature_Configuration.resolver(
        ~fileType="plaintext",
        config,
        Feature_Vim.initial,
      );
    Editor.create(~config, ~buffer=editorBuffer, ());
  };

  let defaultEditorFont = Service_Font.default();

  {
    buffers: Feature_Buffers.empty |> Feature_Buffers.add(initialBuffer),
    bufferHighlights: BufferHighlights.initial,
    bufferRenderers: initialBufferRenderers,
    changelog: Feature_Changelog.initial,
    cli,
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
    input: Feature_Input.initial(defaultKeyBindings),
    quickmenu: None,
    editorFont: defaultEditorFont,
    terminalFont: defaultEditorFont,
    extensions:
      Feature_Extensions.initial(
        ~globalPersistence=extensionGlobalPersistence,
        ~workspacePersistence=extensionWorkspacePersistence,
        ~extensionsFolder,
      ),
    exthost: Feature_Exthost.initial,
    languageSupport: Feature_LanguageSupport.initial,
    lifecycle: Lifecycle.create(),
    logging: Feature_Logging.initial,
    messages: Feature_Messages.initial,
    uiFont: UiFont.default,
    sideBar: Feature_SideBar.initial,
    tokenTheme: TokenTheme.empty,
    iconTheme: IconTheme.create(),
    isQuitting: false,
    languageInfo: Exthost.LanguageInfo.initial,
    grammarRepository: Oni_Syntax.GrammarRepository.empty,
    notifications: Feature_Notification.initial,
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
    registration: Feature_Registration.initial(licenseKeyPersistence),
  };
};
