/*
 * Actions.re
 *
 * Encapsulates actions that can impact the editor state
 */

open State;

module BufferUpdate {
    type t = {
        startLine: int,
        endLine: int,
        lines: list(string),
    };

    let create = (~startLine, ~endLine, ~lines, ()) => {
        startLine,
        endLine,
        lines,
    };
}

type t =
  | BufferUpdate(BufferUpdate.t)
  | ChangeMode(Mode.t)
  | Noop;
