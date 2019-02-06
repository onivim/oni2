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
  | Noop;
