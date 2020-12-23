/*
 * Actions.re
 *
 * Encapsulates actions that can impact the editor state
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Syntax;

[@deriving show({with_path: false})]
type t =
  | Init
  | AutoUpdate(Feature_AutoUpdate.msg)
  | Buffers(Feature_Buffers.msg)
  | Clipboard(Feature_Clipboard.msg)
  | Exthost(Feature_Exthost.msg)
  | Syntax(Feature_Syntax.msg)
  | SignatureHelp(Feature_SignatureHelp.msg)
  | Changelog(Feature_Changelog.msg)
  | Command(string)
  | Commands(Feature_Commands.msg(t))
  | Configuration(Feature_Configuration.msg)
  | ConfigurationParseError(string)
  | ConfigurationReload
  | ConfigurationSet([@opaque] Configuration.t)
  // ConfigurationTransform(fileName, f) where [f] is a configurationTransformer
  // opens the file [fileName] and applies [f] to the loaded JSON.
  | ConfigurationTransform(string, configurationTransformer)
  | Decorations(Feature_Decorations.msg)
  | Diagnostics(Feature_Diagnostics.msg)
  | EditorFont(Service_Font.msg)
  | Help(Feature_Help.msg)
  | Input(Feature_Input.msg)
  | TerminalFont(Service_Font.msg)
  | Extensions(Feature_Extensions.msg)
  | ExtensionBufferUpdateQueued({triggerKey: option(string)})
  | FileChanged(Service_FileWatcher.event)
  | FileSystem(Feature_FileSystem.msg)
  | KeyBindingsSet([@opaque] list(Feature_Input.Schema.resolvedKeybinding))
  // Reload keybindings from configuration
  | KeyBindingsReload
  | KeyBindingsParseError(string)
  | KeybindingInvoked({command: string})
  | KeyDown(EditorInput.KeyPress.t, [@opaque] Revery.Time.t)
  | KeyUp(EditorInput.KeyPress.t, [@opaque] Revery.Time.t)
  | Logging(Feature_Logging.msg)
  | TextInput(string, [@opaque] Revery.Time.t)
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
  | Editor({
      scope: EditorScope.t,
      msg: Feature_Editor.msg,
    })
  | FilesDropped({paths: list(string)})
  | FileExplorer(Feature_Explorer.msg)
  | LanguageSupport(Feature_LanguageSupport.msg)
  | MenuBar(Feature_MenuBar.msg)
  | QuickmenuPaste(string)
  | QuickmenuShow(quickmenuVariant)
  | QuickmenuInput(string)
  | QuickmenuInputMessage(Component_InputText.msg)
  | QuickmenuCommandlineUpdated(string, int)
  | QuickmenuUpdateRipgrepProgress(progress)
  | QuickmenuUpdateFilterProgress([@opaque] array(menuItem), progress)
  | QuickmenuUpdateExtensionItems({
      id: int,
      items: list(Exthost.QuickOpen.Item.t),
    })
  | QuickmenuSearch(string)
  | QuickmenuClose
  | ListFocus(int)
  | ListFocusUp
  | ListFocusDown
  | ListSelect
  | ListSelectBackground
  | NewBuffer({direction: [ | `Current | `Horizontal | `Vertical | `NewTab]})
  | OpenBufferById({
      bufferId: int,
      direction: [ | `Current | `Horizontal | `Vertical | `NewTab],
    })
  | OpenFileByPath(
      string,
      option([ | `Current | `Horizontal | `Vertical | `NewTab]),
      option(CharacterPosition.t),
    )
  | OpenConfigFile(string)
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
  | SearchSetHighlights(int, list(ByteRange.t))
  | SearchClearHighlights(int)
  | SetLanguageInfo([@opaque] Exthost.LanguageInfo.t)
  | SetGrammarRepository([@opaque] Oni_Syntax.GrammarRepository.t)
  | ThemeLoadByPath(string, string)
  | ThemeLoadByName(string)
  | ThemeChanged(string)
  | SetIconTheme([@opaque] IconTheme.t)
  | StatusBar(Feature_StatusBar.msg)
  | TokenThemeLoaded([@opaque] TokenTheme.t)
  | ThemeLoadError(string)
  | EnableZenMode
  | DisableZenMode
  | CopyActiveFilepathToClipboard
  | SCM(Feature_SCM.msg)
  | Search(Feature_Search.msg)
  | SideBar(Feature_SideBar.msg)
  | Sneak(Feature_Sneak.msg)
  | Terminal(Feature_Terminal.msg)
  | Theme(Feature_Theme.msg)
  | Pane(Feature_Pane.msg)
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
  command: unit => t,
  icon: [@opaque] option(IconTheme.IconDefinition.t),
  highlight: list((int, int)),
  handle: option(int),
}
and quickmenuVariant =
  | CommandPalette
  | EditorsPicker
  | FilesPicker
  | OpenBuffersPicker
  | Wildmenu([@opaque] Vim.Types.cmdlineType)
  | ThemesPicker([@opaque] list(Feature_Theme.theme))
  | FileTypesPicker({
      bufferId: int,
      languages:
        list((string, option(Oni_Core.IconTheme.IconDefinition.t))),
    })
  | Extension({
      id: int,
      hasItems: bool,
      resolver: [@opaque] Lwt.u(int),
    })
and progress =
  | Loading
  | InProgress(float)
  | Complete;
