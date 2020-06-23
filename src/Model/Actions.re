/*
 * Actions.re
 *
 * Encapsulates actions that can impact the editor state
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Input;
open Oni_Syntax;
open Oni_Components;

module Ext = Oni_Extensions;
module ContextMenu = Oni_Components.ContextMenu;
module CompletionMeet = Feature_LanguageSupport.CompletionMeet;
module CompletionItem = Feature_LanguageSupport.CompletionItem;
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;
module Diagnostic = Feature_LanguageSupport.Diagnostic;

[@deriving show({with_path: false})]
type t =
  | Init
  | ActivityBar(ActivityBar.action)
  | BufferHighlights(BufferHighlights.action)
  | BufferDisableSyntaxHighlighting(int)
  | BufferEnter({
      id: int,
      fileType: option(string),
      lineEndings: [@opaque] option(Vim.lineEnding),
      filePath: option(string),
      isModified: bool,
      version: int,
      // TODO: This duplication-of-truth is really awkward,
      // but I want to remove it shortly
      buffer: [@opaque] Buffer.t,
    })
  | BufferFilenameChanged({
      id: int,
      newFilePath: option(string),
      newFileType: option(string),
      version: int,
      isModified: bool,
    })
  | BufferUpdate({
      update: [@opaque] BufferUpdate.t,
      oldBuffer: [@opaque] Buffer.t,
      newBuffer: [@opaque] Buffer.t,
      triggerKey: option(string),
    })
  | BufferLineEndingsChanged({
      id: int,
      lineEndings: [@opaque] Vim.lineEnding,
    })
  | BufferRenderer(BufferRenderer.action)
  | BufferSaved(int)
  | BufferSetIndentation(int, [@opaque] IndentationSettings.t)
  | BufferSetModified(int, bool)
  | Syntax(Feature_Syntax.msg)
  | Hover(Feature_Hover.msg)
  | SignatureHelp(Feature_SignatureHelp.msg)
  | Changelog(Feature_Changelog.msg)
  | Command(string)
  | Commands(Feature_Commands.msg(t))
  | CompletionAddItems(
      [@opaque] CompletionMeet.t,
      [@opaque] list(CompletionItem.t),
    )
  | Configuration(Feature_Configuration.msg)
  | ConfigurationParseError(string)
  | ConfigurationReload
  | ConfigurationSet([@opaque] Configuration.t)
  // ConfigurationTransform(fileName, f) where [f] is a configurationTransformer
  // opens the file [fileName] and applies [f] to the loaded JSON.
  | ConfigurationTransform(string, configurationTransformer)
  | DefinitionAvailable(
      int,
      Location.t,
      [@opaque] LanguageFeatures.DefinitionResult.t,
    )
  | EditorFont(Service_Font.msg)
  | TerminalFont(Service_Font.msg)
  | Extension(Extensions.action)
  | ExtensionBufferUpdateQueued({triggerKey: option(string)})
  | FileChanged(Service_FileWatcher.event)
  | References(References.actions)
  | KeyBindingsSet([@opaque] Keybindings.t)
  // Reload keybindings from configuration
  | KeyBindingsReload
  | KeyBindingsParseError(string)
  | KeybindingInvoked({command: string})
  | KeyDown([@opaque] EditorInput.KeyPress.t, [@opaque] Revery.Time.t)
  | KeyUp([@opaque] EditorInput.KeyPress.t, [@opaque] Revery.Time.t)
  | TextInput([@opaque] string, [@opaque] Revery.Time.t)
  | HoverShow
  | ContextMenuOverlayClicked
  | DiagnosticsHotKey
  | DiagnosticsSet(Uri.t, string, [@opaque] list(Diagnostic.t))
  | DiagnosticsClear(string)
  | SelectionChanged([@opaque] VisualRange.t)
  | DisableKeyDisplayer
  | EnableKeyDisplayer
  | KeyboardInput(string)
  | WindowTitleSet(string)
  | EditorGroupSelected(int)
  | EditorGroupSizeChanged({
      id: int,
      width: int,
      height: int,
    })
  | EditorCursorMove(Feature_Editor.EditorId.t, [@opaque] list(Vim.Cursor.t))
  | EditorSizeChanged({
      id: Feature_Editor.EditorId.t,
      pixelWidth: int,
      pixelHeight: int,
    })
  | EditorScrollToLine(Feature_Editor.EditorId.t, int)
  | EditorScrollToColumn(Feature_Editor.EditorId.t, int)
  | EditorTabClicked(int)
  | Formatting(Feature_Formatting.msg)
  | ViewCloseEditor(int)
  | Notification(Feature_Notification.msg)
  | ExtMessageReceived({
      severity: [@opaque] Exthost.Msg.MessageService.severity,
      message: string,
      extensionId: option(string),
    })
  | Editor({
      editorId: int,
      msg: Feature_Editor.msg,
    })
  | FilesDropped({paths: list(string)})
  | FileExplorer(FileExplorer.action)
  | LanguageFeature(LanguageFeatures.action)
  | QuickmenuShow(quickmenuVariant)
  | QuickmenuInput(string)
  | QuickmenuInputClicked(Selection.t)
  | QuickmenuCommandlineUpdated(string, int)
  | QuickmenuUpdateRipgrepProgress(progress)
  | QuickmenuUpdateFilterProgress([@opaque] array(menuItem), progress)
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
      option(Location.t),
    )
  | OpenConfigFile(string)
  | QuitBuffer([@opaque] Vim.Buffer.t, bool)
  | Quit(bool)
  // ReallyQuitting is dispatched when we've decided _for sure_
  // to quit the app. This gives subscriptions the chance to clean up.
  | ReallyQuitting
  | RegisterQuitCleanup(unit => unit)
  | SearchSetHighlights(int, list(Range.t))
  | SearchClearHighlights(int)
  | SetLanguageInfo([@opaque] Ext.LanguageInfo.t)
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
  | Sneak(Feature_Sneak.msg)
  | Terminal(Feature_Terminal.msg)
  | Theme(Feature_Theme.msg)
  | PaneTabClicked(Pane.pane)
  | PaneCloseButtonClicked
  | VimDirectoryChanged(string)
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
  | NewTextContentProvider({
      handle: int,
      scheme: string,
    })
  | LostTextContentProvider({handle: int})
  | Modals(Feature_Modals.msg)
  // "Internal" effect action, see TitleStoreConnector
  | SetTitle(string)
  | GotOriginalUri({
      bufferId: int,
      uri: Uri.t,
    })
  | GotOriginalContent({
      bufferId: int,
      lines: [@opaque] array(string),
    })
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
}
and quickmenuVariant =
  | CommandPalette
  | EditorsPicker
  | FilesPicker
  | Wildmenu([@opaque] Vim.Types.cmdlineType)
  | ThemesPicker
  | DocumentSymbols
and progress =
  | Loading
  | InProgress(float)
  | Complete;
