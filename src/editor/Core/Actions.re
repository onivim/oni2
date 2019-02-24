/*
 * Actions.re
 *
 * Encapsulates actions that can impact the editor state
 */

open Types;

type t =
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
  | CommandlineShow(Commandline.t)
  | CommandlineHide(Commandline.t)
  | CommandlineUpdate((int, int))
  | WildmenuShow(Wildmenu.t)
  | WildmenuHide(Wildmenu.t)
  | WildmenuSelected(int)
  | EditorScroll(int)
  | EditorScrollToCursorCentered
  | EditorScrollToCursorTop
  | EditorScrollToCursorBottom
  | EditorMoveCursorToTop(Cursor.move)
  | EditorMoveCursorToMiddle(Cursor.move)
  | EditorMoveCursorToBottom(Cursor.move)
  | Noop;
