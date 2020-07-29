/*
 * BufferHighlights.rei
 *
 * Per-buffer highlight info
 */
open EditorCoreTypes;

type t;

let initial: t;

let setSearchHighlights: (int, list(Range.t), t) => t;
let clearSearchHighlights: (int, t) => t;

let getHighlightsByLine: (~bufferId: int, ~line: Index.t, t) => list(Range.t);
let getHighlights: (~bufferId: int, t) => list(Index.t);
