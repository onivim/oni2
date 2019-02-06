/*
 * Actions.re
 *
 * Encapsulates actions that can impact the editor state
 */

open State;
open Types;

type t =
  | ChangeMode(Mode.t)
  | SetEditorFont(EditorFont.t)
  | Noop;
