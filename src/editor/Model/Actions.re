/*
 * Actions.re
 *
 * Encapsulates actions that can impact the editor state
 */

open Oni_Core.Types;
open Oni_Extensions;

type t =
  | Init
  | Tick
  | BufferDelete(BufferNotification.t)
  | BufferEnter(BufferNotification.t)
  | BufferUpdate(BufferUpdate.t)
  | BufferWritePost(BufferNotification.t)
  | TablineUpdate(Tabline.tabs)
  | TextChanged(TextChanged.t)
  | TextChangedI(TextChanged.t)
  | ChangeMode(Mode.t)
  | CursorMove(BufferPosition.t)
  | SetEditorFont(EditorFont.t)
  | SetEditorSize(EditorSize.t)
  | RecalculateEditorView
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
  | MenuOpen(MenuCreationInfo.t)
  | MenuUpdate(list(MenuCommand.t))
  | MenuSetDispose(unit => unit)
  | MenuClose
  | MenuSelect
  | MenuPosition(int)
  | OpenFile(string)
  | OpenConfigFile(string)
  | SetInputControlMode(Input.controlMode)
  | StatusBarAddItem(StatusBarModel.Item.t)
  | StatusBarDisposeItem(int)
  | Noop;
