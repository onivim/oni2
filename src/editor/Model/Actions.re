/*
 * Actions.re
 *
 * Encapsulates actions that can impact the editor state
 */

open Oni_Core.Types;
open Oni_Extensions;

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
  | CommandlineShow(commandline)
  | CommandlineHide(commandline)
  | CommandlineUpdate((int, int))
  | WildmenuShow(wildmenu)
  | WildmenuHide(wildmenu)
  | WildmenuSelected(int)
  | EditorScroll(int)
  | EditorScrollToCursorCentered
  | EditorScrollToCursorTop
  | EditorScrollToCursorBottom
  | EditorMoveCursorToTop(Cursor.move)
  | EditorMoveCursorToMiddle(Cursor.move)
  | EditorMoveCursorToBottom(Cursor.move)
  | SyntaxHighlightColorMap(ColorMap.t)
  | SyntaxHighlightTokens(TextmateClient.TokenizationResult.t)
  | Noop;
