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
      metadata: [@opaque] Vim.BufferMetadata.t,
      fileType: option(string),
      lineEndings: [@opaque] option(Vim.lineEnding),
    })
  | BufferUpdate({
      update: [@opaque] BufferUpdate.t,
      oldBuffer: [@opaque] Buffer.t,
      newBuffer: [@opaque] Buffer.t,
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
  | ModeChanged([@opaque] Vim.Mode.t)
  | ContextMenuOverlayClicked
  | ContextMenuItemSelected(ContextMenu.item(t))
  | DiagnosticsHotKey
  | DiagnosticsSet(Uri.t, string, [@opaque] list(Diagnostic.t))
  | DiagnosticsClear(string)
  | SelectionChanged([@opaque] VisualRange.t)
  | RecalculateEditorView([@opaque] option(Buffer.t))
  | DisableKeyDisplayer
  | EnableKeyDisplayer
  | KeyboardInput(string)
  | WindowTitleSet(string)
  | EditorGroupSelected(int)
  | EditorGroupAdd(EditorGroup.t)
  | EditorGroupSizeChanged({
      id: int,
      width: int,
      height: int,
    })
  | EditorCursorMove(Feature_Editor.EditorId.t, [@opaque] list(Vim.Cursor.t))
  | EditorSetScroll(Feature_Editor.EditorId.t, float)
  | EditorSizeChanged({
      id: Feature_Editor.EditorId.t,
      pixelWidth: int,
      pixelHeight: int,
    })
  | EditorScroll(Feature_Editor.EditorId.t, float)
  | EditorScrollToLine(Feature_Editor.EditorId.t, int)
  | EditorScrollToColumn(Feature_Editor.EditorId.t, int)
  | Notification(Feature_Notification.msg)
  | ExtMessageReceived({
      severity: [ | `Ignore | `Info | `Warning | `Error],
      message: string,
      extensionId: option(string),
    })
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
  | AddSplit([ | `Horizontal | `Vertical], int)
  | RemoveSplit(int)
  | OpenConfigFile(string)
  | QuitBuffer([@opaque] Vim.Buffer.t, bool)
  | Quit(bool)
  // ReallyQuitting is dispatched when we've decided _for sure_
  // to quit the app. This gives subscriptions the chance to clean up.
  | ReallyQuitting
  | RegisterQuitCleanup(unit => unit)
  | SearchClearMatchingPair(int)
  | SearchSetMatchingPair(int, Location.t, Location.t)
  | SearchSetHighlights(int, list(Range.t))
  | SearchClearHighlights(int)
  | SetLanguageInfo([@opaque] Ext.LanguageInfo.t)
  | ThemeLoadByPath(string, string)
  | ThemeLoadByName(string)
  | ThemeChanged(string)
  | SetIconTheme([@opaque] IconTheme.t)
  | StatusBarAddItem([@opaque] StatusBarModel.Item.t)
  | StatusBarDisposeItem(int)
  | StatusBar(StatusBarModel.action)
  | TokenThemeLoaded([@opaque] TokenTheme.t)
  | ThemeLoadError(string)
  | ViewCloseEditor(int)
  | ViewSetActiveEditor(int)
  | EnableZenMode
  | DisableZenMode
  | CopyActiveFilepathToClipboard
  | SCM(Feature_SCM.msg)
  | SearchStart
  | SearchHotkey
  | Search(Feature_Search.msg)
  | Sneak(Sneak.action)
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
  | WindowMinimized
  | WindowRestored
  | WindowCloseBlocked
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
