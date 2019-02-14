/*
 * Actions.re
 *
 * Encapsulates actions that can impact the editor state
 */

open Types;

type t =
  | BufferUpdate(BufferUpdate.t)
  | ChangeMode(Mode.t)
  | CursorMove(BufferPosition.t)
  | SetEditorFont(EditorFont.t)
  | SetEditorSize(EditorSize.t)
  | CommandlineShow(Commandline.t)
  | CommandlineHide(Commandline.t)
  | WildmenuShow(Wildmenu.t)
  | WildmenuHide(Wildmenu.t)
  | BufferEnter(Buffer.t)
  | WildmenuSelected(int)
  | Noop;
