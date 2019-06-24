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
  | BufferMarkDirty(int)
  | Command(string)
  | ConfigurationReload
  | ConfigurationSet(Configuration.t)
  | ChangeMode(Vim.Mode.t)
  | CursorMove(Position.t)
  | SelectionChanged(VisualRange.t)
  | SetEditorFont(EditorFont.t)
  | SetEditorSize(EditorSize.t)
  | RecalculateEditorView(option(Buffer.t))
  | CommandlineShow(Vim.Types.cmdlineType)
  | CommandlineHide
  | CommandlineUpdate(Vim.Types.cmdline)
  | KeyboardInput(string)
  | WildmenuShow(wildmenu)
  | WildmenuHide(wildmenu)
  | WildmenuSelected(int)
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
  | SetLanguageInfo(LanguageInfo.t)
  | SetIconTheme(IconTheme.t)
  | SetInputControlMode(Input.controlMode)
  | StatusBarAddItem(StatusBarModel.Item.t)
  | StatusBarDisposeItem(int)
  | ViewCloseEditor(int)
  | ViewSetActiveEditor(int)
  | Noop
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
