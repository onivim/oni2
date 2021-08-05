/*
 * Actions.re
 *
 * Encapsulates actions that can impact the editor state
 */

open EditorCoreTypes;
open Oni_Core;

[@deriving show({with_path: false})]
type t =
  | Init
  | AutoUpdate(Feature_AutoUpdate.msg)
  | Buffers(Feature_Buffers.msg)
  | Clipboard(Feature_Clipboard.msg)
  | Exthost(Feature_Exthost.msg)
  | Syntax(Feature_Syntax.msg)
  | Changelog(Feature_Changelog.msg)
  | ClientServer(Feature_ClientServer.msg)
  | CommandInvoked({
      command: string,
      arguments: Yojson.Safe.t,
    })
  | Commands(Feature_Commands.msg(t))
  | Configuration(Feature_Configuration.msg)
  | ContextMenu(Feature_ContextMenu.msg)
  | Decorations(Feature_Decorations.msg)
  | Diagnostics(Feature_Diagnostics.msg)
  | EditorFont(Service_Font.msg)
  | Help(Feature_Help.msg)
  | Input(Feature_Input.msg)
  | Keyboard(Feature_Keyboard.msg)
  | Extensions(Feature_Extensions.msg)
  | ExtensionBufferUpdateQueued({triggerKey: option(string)})
  | FileChanged(Service_FileWatcher.event)
  | FileSystem(Feature_FileSystem.msg)
  | KeybindingInvoked({
      command: string,
      arguments: Yojson.Safe.t,
    })
  | KeyDown({
      key: EditorInput.KeyCandidate.t,
      scancode: int,
      time: [@opaque] Revery.Time.t,
    })
  | TextInput(string, [@opaque] Revery.Time.t)
  | KeyUp({
      scancode: int,
      time: [@opaque] Revery.Time.t,
    })
  | KeyTimeout
  | Logging(Feature_Logging.msg)
  // TODO: This should be a function call - wired up from an input feature
  // directly to the consumer of the keyboard action.
  // In addition, in the 'not-is-text' case, we should strongly type the keys.
  | KeyboardInput({
      isText: bool,
      input: string,
    })
  | WindowTitleSet(string)
  | EditorGroupSizeChanged({
      id: int,
      width: int,
      height: int,
    })
  | EditorSizeChanged({
      id: Feature_Editor.EditorId.t,
      pixelWidth: int,
      pixelHeight: int,
    })
  | Notification(Feature_Notification.msg)
  | Messages(Feature_Messages.msg)
  | Output(Feature_Output.msg)
  | Editor({
      scope: EditorScope.t,
      msg: Feature_Editor.msg,
    })
  | FilesDropped({paths: list(string)})
  | FileExplorer(Feature_Explorer.msg)
  | LanguageSupport(Feature_LanguageSupport.msg)
  | MenuBar(Feature_MenuBar.msg)
  | Quickmenu(Feature_Quickmenu.msg)
  | QuickmenuPaste(string)
  | QuickmenuShow(quickmenuVariant)
  | QuickmenuInput(string)
  | QuickmenuInputMessage(Component_InputText.msg)
  | QuickmenuCommandlineUpdated(string, int)
  | QuickmenuUpdateRipgrepProgress(progress)
  | QuickmenuUpdateFilterProgress([@opaque] array(menuItem), progress)
  | QuickmenuSearch(string)
  | QuickmenuClose
  | QuickOpen(Feature_QuickOpen.msg)
  | ListFocus(int)
  | ListFocusUp
  | ListFocusDown
  | ListSelect({direction: SplitDirection.t})
  | ListSelectBackground
  | NewBuffer({direction: SplitDirection.t})
  | OpenBufferById({
      bufferId: int,
      direction: SplitDirection.t,
    })
  | OpenFileByPath(string, SplitDirection.t, option(CharacterPosition.t))
  | PreviewFileByPath(string, SplitDirection.t, option(CharacterPosition.t))
  | Pasted({
      rawText: string,
      isMultiLine: bool,
      lines: array(string),
    })
  | Registers(Feature_Registers.msg)
  | Registration(Feature_Registration.msg)
  | QuitBuffer([@opaque] Vim.Buffer.t, bool)
  | Quit(bool)
  // ReallyQuitting is dispatched when we've decided _for sure_
  // to quit the app. This gives subscriptions the chance to clean up.
  | ReallyQuitting
  | RegisterQuitCleanup(unit => unit)
  | SearchClearHighlights(int)
  | SetGrammarRepository([@opaque] Oni_Syntax.GrammarRepository.t)
  | SetIconTheme([@opaque] IconTheme.t)
  | StatusBar(Feature_StatusBar.msg)
  | SCM(Feature_SCM.msg)
  | Search(Feature_Search.msg)
  | SideBar(Feature_SideBar.msg)
  | Sneak(Feature_Sneak.msg)
  | Snippets(Feature_Snippets.msg)
  | Terminal(Feature_Terminal.msg)
  | Theme(Feature_Theme.msg)
  | Pane(Feature_Pane.msg(t))
  | VimExecuteCommand({
      allowAnimation: bool,
      command: string,
    })
  | VimMessageReceived({
      priority: [@opaque] Vim.Types.msgPriority,
      title: string,
      message: string,
    })
  | WindowFocusGained
  | WindowFocusLost
  | WindowMaximized
  | WindowFullscreen
  | WindowMinimized
  | WindowRestored
  | Workspace(Feature_Workspace.msg)
  | TitleBar(Feature_TitleBar.msg)
  | WindowCloseBlocked
  | Layout(Feature_Layout.msg)
  | WriteFailure
  | Modals(Feature_Modals.msg)
  | Vim(Feature_Vim.msg)
  | TabPage(Vim.TabPage.effect)
  | Yank({range: [@opaque] VisualRange.t})
  | Zen(Feature_Zen.msg)
  | Zoom(Feature_Zoom.msg)
  // TEMPORARY imperative actions
  | SynchronizeExperimentalViml(list(string))
  | Noop
and command = {
  commandCategory: option(string),
  commandName: string,
  commandAction: t,
  commandEnabled: unit => bool,
  commandIcon: [@opaque] option(IconTheme.IconDefinition.t),
}
// [configurationTransformer] is a function that modifies configuration json
and configurationTransformer = Yojson.Safe.t => Yojson.Safe.t
and tick = {
  deltaTime: float,
  totalTime: float,
}
and menuItem = {
  category: option(string),
  name: string,
  command: option(SplitDirection.t) => t,
  icon: [@opaque] option(IconTheme.IconDefinition.t),
  highlight: list((int, int)),
}
and quickmenuVariant =
  | CommandPalette
  | EditorsPicker
  | FilesPicker
  | OpenBuffersPicker
  | Wildmenu([@opaque] Vim.Types.cmdlineType)
and progress =
  | Loading
  | InProgress(float)
  | Complete;
