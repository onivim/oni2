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
  | BufferDelete(int)
  | BufferEnter(BufferMetadata.t)
  | BufferUpdate(BufferUpdate.t)
  | BufferSaved(BufferMetadata.t)
  | BufferMarkDirty(int)
  | ConfigurationReload
  | ConfigurationSet(Configuration.t)
  | ChangeMode(Mode.t)
  | CursorMove(Position.t)
  | SelectionChanged(VisualRange.t)
  | SetEditorFont(EditorFont.t)
  | SetEditorSize(EditorSize.t)
  | RecalculateEditorView(option(Buffer.t))
  | CommandlineShow(commandline)
  | CommandlineHide(commandline)
  | CommandlineUpdate((int, int))
  | KeyboardInput(string)
  | WildmenuShow(wildmenu)
  | WildmenuHide(wildmenu)
  | WildmenuSelected(int)
  | EditorScroll(float)
  | EditorScrollToCursorCentered
  | EditorScrollToCursorTop
  | EditorScrollToCursorBottom
  | EditorMoveCursorToTop(Cursor.move)
  | EditorMoveCursorToMiddle(Cursor.move)
  | EditorMoveCursorToBottom(Cursor.move)
  | SyntaxHighlightColorMap(ColorMap.t)
  | SyntaxHighlightTokens(TextmateClient.TokenizationResult.t)
  | MenuSearch(string)
  | MenuOpen(menuCreator)
  | MenuUpdate(list(menuCommand))
  | MenuSetDispose(unit => unit)
  | MenuClose
  | MenuSelect
  | MenuNextItem
  | MenuPreviousItem
  | MenuPosition(int)
  | CloseFileById(int)
  | OpenFileByPath(string)
  | OpenFileById(int)
  | AddRightDock(WindowManager.dock)
  | AddLeftDock(WindowManager.dock)
  | AddSplit(WindowManager.splitMetadata)
  | RemoveSplit(int)
  | OpenConfigFile(string)
  | QuickOpen
  | SetLanguageInfo(LanguageInfo.t)
  | SetIconTheme(IconTheme.t)
  | SetInputControlMode(Input.controlMode)
  | StatusBarAddItem(StatusBarModel.Item.t)
  | StatusBarDisposeItem(int)
  | Noop
and menuCommand = {
  category: option(string),
  name: string,
  command: unit => t,
  icon: option(string),
}
and menuSetItems = list(menuCommand) => unit
and menuCreationFunction = menuSetItems => unit
and menuDisposeFunction = unit => unit
and menuCreator = menuSetItems => menuDisposeFunction;
