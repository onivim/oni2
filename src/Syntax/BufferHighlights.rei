/*
 * BufferHighlights.rei
 *
 * Per-buffer highlight info
 */
open EditorCoreTypes;

type t;

let initial: t;

let setSearchHighlights: (int, list(ByteRange.t), t) => t;
let clearSearchHighlights: (int, t) => t;

let getHighlightsByLine:
  (~bufferId: int, ~line: EditorCoreTypes.LineNumber.t, t) =>
  list(ByteRange.t);
let getHighlights: (~bufferId: int, t) => list(LineNumber.t);
