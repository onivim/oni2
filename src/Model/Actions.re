/*
 * Actions.re
 *
 * Encapsulates actions that can impact the editor state
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Input;
open Oni_Syntax;

module ContextMenu = Oni_Components.ContextMenu;
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;
module Diagnostic = Feature_LanguageSupport.Diagnostic;

[@deriving show({with_path: false})]
type t =
  | Init
  | ActivityBar(ActivityBar.action)
  | Buffers(Feature_Buffers.msg)
  | BufferRenderer(BufferRenderer.action)
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
  | EditorFont(Service_Font.msg)
  | TerminalFont(Service_Font.msg)
  | Extensions(Feature_Extensions.msg)
  | ExtensionBufferUpdateQueued({triggerKey: option(string)})
  | FileChanged(Service_FileWatcher.event)
  | KeyBindingsSet([@opaque] Keybindings.t)
  // Reload keybindings from configuration
  | KeyBindingsReload
  | KeyBindingsParseError(string)
  | KeybindingInvoked({command: string})
  | KeyDown([@opaque] EditorInput.KeyPress.t, [@opaque] Revery.Time.t)
  | KeyUp([@opaque] EditorInput.KeyPress.t, [@opaque] Revery.Time.t)
  | TextInput([@opaque] string, [@opaque] Revery.Time.t)
  | ContextMenuOverlayClicked
  | DiagnosticsHotKey
  | DiagnosticsSet(Uri.t, string, [@opaque] list(Diagnostic.t))
  | DiagnosticsClear(string)
  | DisableKeyDisplayer
  | EnableKeyDisplayer
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
  | FileExplorer(FileExplorer.action)
  | LanguageFeature(LanguageFeatures.action)
  | LanguageSupport(Feature_LanguageSupport.msg)
  | QuickmenuPaste(string)
  | QuickmenuShow(quickmenuVariant)
  | QuickmenuInput(string)
  | QuickmenuInputMessage(Feature_InputText.msg)
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
  | OpenFileByPath(
      string,
      option([ | `Horizontal | `Vertical]),
      option(CharacterPosition.t),
    )
  | OpenFileInNewLayout(string)
  | BufferOpened(string, option(CharacterPosition.t), int)
  | BufferOpenedForLayout(int)
  | OpenConfigFile(string)
  | Pasted({
      rawText: string,
      isMultiLine: bool,
      lines: array(string),
    })
  | Registers(Feature_Registers.msg)
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
  | SearchStart
  | SearchHotkey
  | Search(Feature_Search.msg)
  | SideBar(Feature_SideBar.msg)
  | Sneak(Feature_Sneak.msg)
  | Terminal(Feature_Terminal.msg)
  | Theme(Feature_Theme.msg)
  | Pane(Feature_Pane.msg)
  | PaneTabClicked(Feature_Pane.pane)
  | PaneCloseButtonClicked
  | DirectoryChanged(string)
  | VimExecuteCommand(string)
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
  | TitleBar(Feature_TitleBar.msg)
  | WindowCloseBlocked
  | Layout(Feature_Layout.msg)
  | WriteFailure
  | Modals(Feature_Modals.msg)
  // "Internal" effect action, see TitleStoreConnector
  | SetTitle(string)
  | NewDecorationProvider({
      handle: int,
      label: string,
    })
  | LostDecorationProvider({handle: int})
  | DecorationsChanged({
      handle: int,
      uris: list(Uri.t),
    })
  | GotDecorations({
      handle: int,
      uri: Uri.t,
      decorations: list(Decoration.t),
    })
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
  | Wildmenu([@opaque] Vim.Types.cmdlineType)
  | ThemesPicker([@opaque] list(Feature_Theme.theme))
  | FileTypesPicker({
      bufferId: int,
      languages:
        list((string, option(Oni_Core.IconTheme.IconDefinition.t))),
    })
  | DocumentSymbols
  | Extension({
      id: int,
      hasItems: bool,
      resolver: [@opaque] Lwt.u(int),
    })
and progress =
  | Loading
  | InProgress(float)
  | Complete;
