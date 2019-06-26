/*
 * Actions.re
 *
 * Encapsulates actions that can impact the editor state
 */

open Oni_Core;
open Oni_Core.Types;
open Oni_Extensions;

type t =
  | Init
  | Tick
  | BufferEnter(Vim.BufferMetadata.t)
  | BufferUpdate(BufferUpdate.t)
  | BufferSaved(Vim.BufferMetadata.t)
  | BufferSetIndentation(int, IndentationSettings.t)
  | BufferMarkDirty(int)
  | Command(string)
  | ConfigurationReload
  | ConfigurationSet(Configuration.t)
  | ChangeMode(Vim.Mode.t)
  | CursorMove(Position.t)
  | SelectionChanged(VisualRange.t)
  | SetEditorFont(EditorFont.t)
  | RecalculateEditorView(option(Buffer.t))
  | CommandlineShow(Vim.Types.cmdlineType)
  | CommandlineHide
  | CommandlineUpdate(Vim.Types.cmdline)
  | KeyboardInput(string)
  | WildmenuShow(wildmenu)
  | WildmenuHide(wildmenu)
  | WildmenuSelected(int)
  | EditorGroupAdd(editorGroup)
  | EditorGroupSetSize(int, EditorSize.t)
  | EditorScroll(float)
  | EditorScrollToLine(int)
  | SyntaxHighlightColorMap(ColorMap.t)
  | SyntaxHighlightTokens(TextmateClient.TokenizationResult.t)
  | OpenExplorer(string)
  | SetExplorerTree(UiTree.t)
  | UpdateExplorerNode(UiTree.t, UiTree.t)
  | MenuSearch(string)
  | MenuOpen(menuCreator)
  | MenuUpdate(list(menuCommand))
  | MenuSetDispose(unit => unit)
  | MenuClose
  | MenuSelect
  | MenuNextItem
  | MenuPreviousItem
  | MenuPosition(int)
  | OpenFileByPath(string)
  | RegisterDockItem(WindowManager.dock)
  | RemoveDockItem(WindowManager.docks)
  | AddDockItem(WindowManager.docks)
  | AddSplit(WindowManager.splitMetadata)
  | RemoveSplit(int)
  | OpenConfigFile(string)
  | QuickOpen
  | Quit
  | RegisterQuitCleanup(unit => unit)
  | SearchClearMatchingPair(int)
  | SearchSetMatchingPair(int, Position.t, Position.t)
  | SearchSetHighlights(int, list(Range.t))
  | SetLanguageInfo(LanguageInfo.t)
  | SetIconTheme(IconTheme.t)
  | SetInputControlMode(Input.controlMode)
  | StatusBarAddItem(StatusBarModel.Item.t)
  | StatusBarDisposeItem(int)
  | ViewCloseEditor(int)
  | ViewSetActiveEditor(int)
  | Noop
and editor = {
  editorId: int,
  bufferId: int,
  scrollX: float,
  scrollY: float,
  minimapMaxColumnWidth: int,
  minimapScrollY: float,
  /*
   * The maximum line visible in the view.
   * TODO: This will be dependent on line-wrap settings.
   */
  maxLineLength: int,
  viewLines: int,
  cursorPosition: Position.t,
  selection: VisualRange.t,
}
and editorMetrics = {
  pixelWidth: int,
  pixelHeight: int,
  lineHeight: float,
  characterWidth: float,
}
and editorGroup = {
  editorGroupId: int,
  activeEditorId: option(int),
  editors: IntMap.t(editor),
  bufferIdToEditorId: IntMap.t(int),
  reverseTabOrder: list(int),
  metrics: editorMetrics,
}
and menuCommand = {
  category: option(string),
  name: string,
  command: unit => t,
  icon: option(IconTheme.IconDefinition.t),
}
and menuSetItems = list(menuCommand) => unit
and menuCreationFunction = menuSetItems => unit
and menuDisposeFunction = unit => unit
and menuCreator = menuSetItems => menuDisposeFunction;
