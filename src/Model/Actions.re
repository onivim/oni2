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

type initOptions = {syntaxHighlightingEnabled: bool};

[@deriving show({with_path: false})]
type t =
  | Init([@opaque] initOptions)
  | ActivityBar(ActivityBar.action)
  | BufferHighlights(BufferHighlights.action)
  | BufferDisableSyntaxHighlighting(int)
  | BufferEnter([@opaque] Vim.BufferMetadata.t, option(string))
  | BufferUpdate([@opaque] BufferUpdate.t)
  | BufferRenderer(BufferRenderer.action)
  | BufferSaved(int)
  | BufferSetIndentation(int, [@opaque] IndentationSettings.t)
  | BufferSetModified(int, bool)
  | BufferSyntaxHighlights([@opaque] list(Protocol.TokenUpdate.t))
  | SyntaxServerStarted([@opaque] Oni_Syntax_Client.t)
  | SyntaxServerClosed
  | Command(string)
  | CommandsRegister(list(command))
  // Execute a contribute command, from an extension
  | CommandExecuteContributed(string)
  | CompletionAddItems(
      [@opaque] CompletionMeet.t,
      [@opaque] list(CompletionItem.t),
    )
  | ConfigurationReload
  | ConfigurationSet([@opaque] Configuration.t)
  // ConfigurationTransform(fileName, f) where [f] is a configurationTransformer
  // opens the file [fileName] and applies [f] to the loaded JSON.
  | ConfigurationTransform(string, configurationTransformer)
  | DarkModeSet(bool)
  | DefinitionAvailable(
      int,
      Location.t,
      [@opaque] LanguageFeatures.DefinitionResult.t,
    )
  | Extension(Extensions.action)
  | References(References.actions)
  | KeyBindingsSet([@opaque] Keybindings.t)
  // Reload keybindings from configuration
  | KeyBindingsReload
  | KeyDown([@opaque] Revery.Key.KeyEvent.t)
  | KeyUp([@opaque] Revery.Key.KeyEvent.t)
  | TextInput([@opaque] Revery.Events.textInputEvent)
  | HoverShow
  | ChangeMode([@opaque] Vim.Mode.t)
  | ContextMenuOverlayClicked
  | ContextMenuItemSelected(ContextMenu.item(t))
  | DiagnosticsHotKey
  | DiagnosticsSet(Uri.t, string, [@opaque] list(Diagnostic.t))
  | DiagnosticsClear(string)
  | SelectionChanged([@opaque] VisualRange.t)
  // LoadEditorFont is the request to load a new font
  // If successful, a SetEditorFont action will be dispatched.
  | LoadEditorFont(string, float)
  | SetEditorFont([@opaque] EditorFont.t)
  | RecalculateEditorView([@opaque] option(Buffer.t))
  | NotifyKeyPressed(float, string)
  | DisableKeyDisplayer
  | EnableKeyDisplayer
  | KeyboardInput(string)
  | WindowSetActive(int, int)
  | WindowTitleSet(string)
  | WindowTreeSetSize(int, int)
  | EditorGroupAdd(EditorGroup.t)
  | EditorGroupSetSize(int, EditorSize.t)
  | EditorCursorMove(Feature_Editor.EditorId.t, [@opaque] list(Vim.Cursor.t))
  | EditorSetScroll(Feature_Editor.EditorId.t, float)
  | EditorScroll(Feature_Editor.EditorId.t, float)
  | EditorScrollToLine(Feature_Editor.EditorId.t, int)
  | EditorScrollToColumn(Feature_Editor.EditorId.t, int)
  | ShowNotification(Notification.t)
  | HideNotification(Notification.t)
  | ClearNotifications
  | FileExplorer(FileExplorer.action)
  | LanguageFeature(LanguageFeatures.action)
  | QuickmenuShow(quickmenuVariant)
  | QuickmenuInput(string)
  | QuickmenuInputClicked(Oni_Components.Selection.t)
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
  | OpenFileByPath(string, option(WindowTree.direction), option(Location.t))
  | AddSplit(WindowTree.direction, WindowTree.split)
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
  | SetIconTheme([@opaque] IconTheme.t)
  | SetTokenTheme([@opaque] TokenTheme.t)
  | SetColorTheme([@opaque] Theme.t)
  | StatusBarAddItem([@opaque] StatusBarModel.Item.t)
  | StatusBarDisposeItem(int)
  | StatusBar(StatusBarModel.action)
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
  | PaneTabClicked(Pane.pane)
  | VimDirectoryChanged(string)
  | WindowFocusGained
  | WindowFocusLost
  | WindowMaximized
  | WindowMinimized
  | WindowRestored
  | WindowCloseBlocked
  | WindowCloseDiscardConfirmed
  | WindowCloseSaveAllConfirmed
  | WindowCloseCanceled
  | NewTextContentProvider({
      handle: int,
      scheme: string,
    })
  | LostTextContentProvider({handle: int})
  | Modal(Modal.msg)
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
