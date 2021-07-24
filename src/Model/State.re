/*
 * State.re
 *
 * Top-level state of the editor
 */

open Oni_Core;

module Commands = GlobalCommands;
let windowCommandCondition =
  "!commandLineFocus && !insertMode && !inQuickOpen || terminalFocus"
  |> WhenExpr.parse;

let isMacCondition = "isMac" |> WhenExpr.parse;
let defaultKeyBindings =
  Feature_Input.Schema.[
    bind(
      ~key="<UP>",
      ~command=Commands.List.focusUp.id,
      ~condition="listFocus || textInputFocus" |> WhenExpr.parse,
    ),
    bind(
      ~key="<DOWN>",
      ~command=Commands.List.focusDown.id,
      ~condition="listFocus || textInputFocus" |> WhenExpr.parse,
    ),
    bind(
      ~key="<RIGHT>",
      ~command=Commands.List.selectBackground.id,
      ~condition="quickmenuCursorEnd" |> WhenExpr.parse,
    ),
  ]
  @ Feature_Search.Contributions.keybindings
  @ Feature_SideBar.Contributions.keybindings
  @ Feature_Keyboard.Contributions.keybindings
  @ Feature_Clipboard.Contributions.keybindings
  @ Feature_Configuration.Contributions.keybindings
  @ Feature_Input.Schema.[
      bind(
        ~key="<C-TAB>",
        ~command=
          Commands.Workbench.Action.openNextRecentlyUsedEditorInGroup.id,
        ~condition="editorTextFocus || terminalFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<C-P>",
        ~command=Commands.Workbench.Action.quickOpen.id,
        ~condition="editorTextFocus || terminalFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<D-P>",
        ~command=Commands.Workbench.Action.quickOpen.id,
        ~condition="isMac" |> WhenExpr.parse,
      ),
      bind(
        ~key="<S-C-P>",
        ~command=Commands.Workbench.Action.showCommands.id,
        ~condition="!isMac" |> WhenExpr.parse,
      ),
      bind(
        ~key="<D-S-P>",
        ~command=Commands.Workbench.Action.showCommands.id,
        ~condition="isMac" |> WhenExpr.parse,
      ),
      bind(
        ~key="<ESC>",
        ~command=Commands.Workbench.Action.closeQuickOpen.id,
        // When in command-line mode, we should let Vim control when it is closed
        // (The <esc> key might not always trigger closing, like if 'insert literal' (Ctrl-v/Ctrl-q) is active)
        ~condition="!commandLineFocus && inQuickOpen" |> WhenExpr.parse,
      ),
      bind(
        ~key="<C-N>",
        ~command=Commands.List.focusDown.id,
        ~condition="listFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<C-P>",
        ~command=Commands.List.focusUp.id,
        ~condition="listFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<D-N>",
        ~command=Commands.List.focusDown.id,
        ~condition="isMac && listFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<D-P>",
        ~command=Commands.List.focusUp.id,
        ~condition="isMac && listFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<TAB>",
        ~command=Commands.List.focusDown.id,
        ~condition="listFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<S-TAB>",
        ~command=Commands.List.focusUp.id,
        ~condition="listFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<C-TAB>",
        ~command=
          Commands.Workbench.Action.quickOpenNavigateNextInEditorPicker.id,
        ~condition="inEditorsPicker" |> WhenExpr.parse,
      ),
      bind(
        ~key="<S-C-TAB>",
        ~command=
          Commands.Workbench.Action.quickOpenNavigatePreviousInEditorPicker.id,
        ~condition="inEditorsPicker" |> WhenExpr.parse,
      ),
      bind(
        ~key="<release>",
        ~command=Commands.List.select.id,
        ~condition="inEditorsPicker" |> WhenExpr.parse,
      ),
    ]
  @ Feature_Registers.Contributions.keybindings
  @ Feature_LanguageSupport.Contributions.keybindings
  @ Feature_Buffers.Contributions.keybindings
  @ Feature_Input.Schema.[
      bind(
        ~key="<CR>",
        ~command=Commands.List.select.id,
        ~condition="listFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<S-CR>",
        ~command=Commands.List.selectVertical.id,
        ~condition="listFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<C-x>",
        ~command=Commands.List.selectHorizontal.id,
        ~condition="listFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<C-t>",
        ~command=Commands.List.selectNewTab.id,
        ~condition="listFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<D-Z>",
        ~command=Commands.undo.id,
        ~condition="isMac && editorTextFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<D-S-Z>",
        ~command=Commands.redo.id,
        ~condition="isMac && editorTextFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<D-S>",
        ~command=Commands.Workbench.Action.Files.save.id,
        ~condition="isMac && editorTextFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<C-S>",
        ~command=Commands.Workbench.Action.Files.save.id,
        ~condition="editorTextFocus" |> WhenExpr.parse,
      ),
      bind(
        ~key="<C-]>",
        ~command=Commands.Editor.Action.indentLines.id,
        ~condition="visualMode" |> WhenExpr.parse,
      ),
      bind(
        ~key="<C-[>",
        ~command=Commands.Editor.Action.outdentLines.id,
        ~condition="visualMode" |> WhenExpr.parse,
      ),
      bind(
        ~key="<D-]>",
        ~command=Commands.Editor.Action.indentLines.id,
        ~condition="isMac && visualMode" |> WhenExpr.parse,
      ),
      bind(
        ~key="<D-[>",
        ~command=Commands.Editor.Action.outdentLines.id,
        ~condition="isMac && visualMode" |> WhenExpr.parse,
      ),
      bind(
        ~key="<TAB>",
        ~command=Commands.indent.id,
        ~condition="visualMode" |> WhenExpr.parse,
      ),
      bind(
        ~key="<S-TAB>",
        ~command=Commands.outdent.id,
        ~condition="visualMode" |> WhenExpr.parse,
      ),
      bind(
        ~key="<C-G>",
        ~command=Feature_Sneak.Commands.start.id,
        ~condition=WhenExpr.Value(True),
      ),
      bind(
        ~key="<ESC>",
        ~command=Feature_Sneak.Commands.stop.id,
        ~condition="sneakMode" |> WhenExpr.parse,
      ),
    ]
  @ Feature_Pane.Contributions.keybindings
  @ Feature_Input.Schema.[
      bind(
        ~key="<D-W>",
        ~command=Feature_Layout.Commands.closeActiveEditor.id,
        ~condition=isMacCondition,
      ),
      bind(
        ~key="<C-PAGEDOWN>",
        ~command=Feature_Layout.Commands.nextEditor.id,
        ~condition=WhenExpr.Value(True),
      ),
      bind(
        ~key="<D-S-]>",
        ~command=Feature_Layout.Commands.nextEditor.id,
        ~condition=isMacCondition,
      ),
      bind(
        ~key="<C-PAGEUP>",
        ~command=Feature_Layout.Commands.previousEditor.id,
        ~condition=WhenExpr.Value(True),
      ),
      bind(
        ~key="<D-S-[>",
        ~command=Feature_Layout.Commands.previousEditor.id,
        ~condition=isMacCondition,
      ),
    ]
  @ Feature_Zoom.Contributions.keybindings
  @ Feature_Terminal.Contributions.keybindings
  //LAYOUT
  @ Feature_Input.Schema.[
      bind(
        ~key="<C-W>H",
        ~command=Feature_Layout.Commands.moveLeft.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><C-H>",
        ~command=Feature_Layout.Commands.moveLeft.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><LEFT>",
        ~command=Feature_Layout.Commands.moveLeft.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>L",
        ~command=Feature_Layout.Commands.moveRight.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><C-L>",
        ~command=Feature_Layout.Commands.moveRight.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>T",
        ~command=Feature_Layout.Commands.moveTopLeft.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><C-T>",
        ~command=Feature_Layout.Commands.moveTopLeft.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>B",
        ~command=Feature_Layout.Commands.moveBottomRight.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><C-B>",
        ~command=Feature_Layout.Commands.moveBottomRight.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>W",
        ~command=Feature_Layout.Commands.cycleForward.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><C-W>",
        ~command=Feature_Layout.Commands.cycleForward.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><S-W>",
        ~command=Feature_Layout.Commands.cycleBackward.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><C-S>",
        ~command=Feature_Layout.Commands.splitHorizontal.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>S",
        ~command=Feature_Layout.Commands.splitHorizontal.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><C-V>",
        ~command=Feature_Layout.Commands.splitVertical.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>V",
        ~command=Feature_Layout.Commands.splitVertical.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><RIGHT>",
        ~command=Feature_Layout.Commands.moveRight.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>K",
        ~command=Feature_Layout.Commands.moveUp.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><C-K>",
        ~command=Feature_Layout.Commands.moveUp.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><UP>",
        ~command=Feature_Layout.Commands.moveUp.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>J",
        ~command=Feature_Layout.Commands.moveDown.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><C-J>",
        ~command=Feature_Layout.Commands.moveDown.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><DOWN>",
        ~command=Feature_Layout.Commands.moveDown.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>R",
        ~command=Feature_Layout.Commands.rotateForward.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><C-R>",
        ~command=Feature_Layout.Commands.rotateForward.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><C-S-R>",
        ~command=Feature_Layout.Commands.rotateBackward.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>-",
        ~command=Feature_Layout.Commands.decreaseVerticalSize.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>+",
        ~command=Feature_Layout.Commands.increaseVerticalSize.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><LT>",
        ~command=Feature_Layout.Commands.increaseHorizontalSize.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><GT>",
        ~command=Feature_Layout.Commands.decreaseHorizontalSize.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>=",
        ~command=Feature_Layout.Commands.resetSizes.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>_",
        ~command=Feature_Layout.Commands.maximizeVertical.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>|",
        ~command=Feature_Layout.Commands.maximizeHorizontal.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>o",
        ~command=Feature_Layout.Commands.toggleMaximize.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><C-Q>",
        ~command=Feature_Layout.Commands.closeActiveSplit.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>q",
        ~command=Feature_Layout.Commands.closeActiveSplit.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W>c",
        ~command=Feature_Layout.Commands.closeActiveSplitUnlessLast.id,
        ~condition=windowCommandCondition,
      ),
      bind(
        ~key="<C-W><C-c>",
        ~command=Feature_Layout.Commands.closeActiveSplitUnlessLast.id,
        ~condition=windowCommandCondition,
      ),
    ]
  @ Component_VimWindows.Contributions.keybindings
  @ Component_VimList.Contributions.keybindings
  @ Component_VimTree.Contributions.keybindings
  @ Feature_Snippets.Contributions.keybindings
  @ Feature_Vim.Contributions.keybindings
  @ Feature_Diagnostics.Contributions.keybindings;

type windowDisplayMode =
  | Minimized
  | Windowed
  | Maximized
  | Fullscreen;

type t = {
  buffers: Feature_Buffers.model,
  bufferRenderers: BufferRenderers.t,
  changelog: Feature_Changelog.model,
  cli: Oni_CLI.t,
  clientServer: Feature_ClientServer.model,
  clipboard: Feature_Clipboard.model,
  colorTheme: Feature_Theme.model,
  commands: Feature_Commands.model(Actions.t),
  config: Feature_Configuration.model,
  decorations: Feature_Decorations.model,
  diagnostics: Feature_Diagnostics.model,
  editorFont: Service_Font.font,
  fileSystem: Feature_FileSystem.model,
  help: Feature_Help.model,
  input: Feature_Input.model,
  keyboard: Feature_Keyboard.model,
  logging: Feature_Logging.model,
  messages: Feature_Messages.model,
  output: Feature_Output.model,
  uiFont: UiFont.t,
  quickmenu: option(Quickmenu.t),
  quickOpen: Feature_QuickOpen.model,
  sideBar: Feature_SideBar.model,
  extensions: Feature_Extensions.model,
  exthost: Feature_Exthost.model,
  iconTheme: IconTheme.t,
  isQuitting: bool,
  languageSupport: Feature_LanguageSupport.model,
  grammarRepository: Oni_Syntax.GrammarRepository.t,
  lifecycle: Lifecycle.t,
  menuBar: Feature_MenuBar.model,
  notifications: Feature_Notification.model,
  proxy: Feature_Proxy.model,
  registers: Feature_Registers.model,
  scm: Feature_SCM.model,
  sneak: Feature_Sneak.model,
  statusBar: Feature_StatusBar.model,
  syntaxHighlights: Feature_Syntax.t,
  terminals: Feature_Terminal.t,
  layout: Feature_Layout.model,
  fileExplorer: Feature_Explorer.model,
  windowIsFocused: bool,
  windowDisplayMode,
  titlebarHeight: float,
  workspace: Feature_Workspace.model,
  zen: Feature_Zen.model,
  // State of the bottom pane
  pane: Feature_Pane.model(t, Actions.t),
  newQuickmenu: Feature_Quickmenu.model(Actions.t),
  searchPane: Feature_Search.model,
  focus: Focus.stack,
  modal: option(Feature_Modals.model),
  snippets: Feature_Snippets.model,
  textContentProviders: list((int, string)),
  titleBar: Feature_TitleBar.model,
  vim: Feature_Vim.model,
  zoom: Feature_Zoom.model,
  autoUpdate: Feature_AutoUpdate.model,
  registration: Feature_Registration.model,
  contextMenu: Feature_ContextMenu.model,
};

let initial =
    (
      ~cli,
      ~initialBuffer,
      ~initialBufferRenderers,
      ~extensionGlobalPersistence,
      ~extensionWorkspacePersistence,
      ~configurationLoader,
      ~keybindingsLoader,
      ~workingDirectory,
      ~maybeWorkspace,
      ~extensionsFolder,
      ~licenseKeyPersistence,
      ~titlebarHeight,
      ~getZoom,
      ~setZoom,
      ~useNativeTitleBar,
    ) => {
  let config =
    Feature_Configuration.initial(
      ~loader=configurationLoader,
      [
        Component_FileExplorer.Contributions.configuration,
        Feature_AutoUpdate.Contributions.configuration,
        Feature_Buffers.Contributions.configuration,
        Feature_Editor.Contributions.configuration,
        Feature_Input.Contributions.configuration,
        Feature_MenuBar.Contributions.configuration,
        Feature_Search.Contributions.configuration,
        Feature_SideBar.Contributions.configuration,
        Feature_Syntax.Contributions.configuration,
        Feature_Terminal.Contributions.configuration,
        Feature_Theme.Contributions.configuration,
        Feature_LanguageSupport.Contributions.configuration,
        Feature_Layout.Contributions.configuration,
        Feature_StatusBar.Contributions.configuration,
        Feature_TitleBar.Contributions.configuration,
        Feature_Vim.Contributions.configuration,
        Feature_Zen.Contributions.configuration,
        Feature_Zoom.Contributions.configuration,
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
    Editor.create(~config, ~buffer=editorBuffer, ~preview=false, ());
  };

  let defaultEditorFont = Service_Font.default();

  {
    buffers: Feature_Buffers.empty |> Feature_Buffers.add(initialBuffer),
    bufferRenderers: initialBufferRenderers,
    changelog: Feature_Changelog.initial,
    cli,
    clientServer: Feature_ClientServer.create(),
    clipboard: Feature_Clipboard.initial,
    colorTheme:
      Feature_Theme.initial([
        Feature_LanguageSupport.Contributions.colors,
        Feature_Terminal.Contributions.colors,
        Feature_Notification.Contributions.colors,
      ]),
    commands: Feature_Commands.initial([]),
    config,
    decorations: Feature_Decorations.initial,
    diagnostics: Feature_Diagnostics.initial,
    input:
      Feature_Input.initial(~loader=keybindingsLoader, defaultKeyBindings),
    keyboard: Feature_Keyboard.initial,
    quickmenu: None,
    editorFont: defaultEditorFont,
    extensions:
      Feature_Extensions.initial(
        ~globalPersistence=extensionGlobalPersistence,
        ~workspacePersistence=extensionWorkspacePersistence,
        ~extensionsFolder,
      ),
    exthost: Feature_Exthost.initial,
    fileSystem: Feature_FileSystem.initial,
    languageSupport: Feature_LanguageSupport.initial,
    lifecycle: Lifecycle.create(),
    logging: Feature_Logging.initial,
    messages: Feature_Messages.initial,
    uiFont: UiFont.default,
    sideBar: Feature_SideBar.initial,
    help: Feature_Help.initial,
    iconTheme: IconTheme.create(),
    isQuitting: false,
    menuBar:
      Feature_MenuBar.initial(
        ~menus=[],
        ~groups=
          [Feature_Workspace.Contributions.menuGroup]
          @ Feature_LanguageSupport.Contributions.menuGroups
          @ Feature_SideBar.Contributions.menuGroups
          @ Feature_Help.Contributions.menuGroups
          @ Feature_Keyboard.Contributions.menuGroups,
      ),
    output: Feature_Output.initial,
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
    titlebarHeight,
    workspace:
      Feature_Workspace.initial(
        ~openedFolder=maybeWorkspace |> Option.map(FpExp.toString),
        workingDirectory,
      ),
    fileExplorer: Feature_Explorer.initial(~rootPath=maybeWorkspace),
    zen:
      Feature_Zen.initial(~isSingleFile=List.length(cli.filesToOpen) == 1),
    pane:
      Feature_Pane.initial(
        [
          Feature_Diagnostics.Contributions.pane
          |> Feature_Pane.Schema.map(
               ~msg=msg => Actions.Diagnostics(msg),
               ~model=state => state.diagnostics,
             ),
          Feature_Notification.Contributions.pane
          |> Feature_Pane.Schema.map(
               ~msg=msg => Actions.Notification(msg),
               ~model=state => state.notifications,
             ),
        ]
        @ (
          Feature_LanguageSupport.Contributions.panes
          |> List.map(
               Feature_Pane.Schema.map(
                 ~msg=msg => Actions.LanguageSupport(msg),
                 ~model=state => state.languageSupport,
               ),
             )
        )
        @ [
          Feature_Output.Contributions.pane
          |> Feature_Pane.Schema.map(
               ~msg=msg => Actions.Output(msg),
               ~model=state => state.output,
             ),
          Feature_Terminal.Contributions.pane
          |> Feature_Pane.Schema.map(
               ~msg=msg => Actions.Terminal(msg),
               ~model=state => state.terminals,
             ),
        ],
      ),
    proxy: Feature_Proxy.default,
    newQuickmenu: Feature_Quickmenu.initial,
    searchPane: Feature_Search.initial,
    focus: Focus.initial,
    modal: None,
    quickOpen: Feature_QuickOpen.initial,
    snippets: Feature_Snippets.initial,
    terminals: Feature_Terminal.initial,
    titleBar: Feature_TitleBar.initial(~useNativeTitleBar),
    textContentProviders: [],
    vim: Feature_Vim.initial,
    zoom: Feature_Zoom.initial(~getZoom, ~setZoom),
    autoUpdate: Feature_AutoUpdate.initial,
    registration: Feature_Registration.initial(licenseKeyPersistence),
    contextMenu: Feature_ContextMenu.initial,
  };
};
