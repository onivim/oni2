/*
 * Actions.re
 *
 * Encapsulates actions that can impact the editor state
 */

open State;

type t =
| ChangeMode(Mode.t)
| Noop
;
