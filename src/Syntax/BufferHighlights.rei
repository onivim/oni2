/*
 * BufferHighlights.rei
 *
 * Per-buffer highlight info
 */
open EditorCoreTypes;

type t;

let initial: t;

let setSearchHighlights: (int, list(CharacterRange.t), t) => t;
let clearSearchHighlights: (int, t) => t;

let getHighlightsByLine:
  (~bufferId: int, ~line: EditorCoreTypes.LineNumber.t, t) =>
  list(CharacterRange.t);
let getHighlights: (~bufferId: int, t) => list(LineNumber.t);
