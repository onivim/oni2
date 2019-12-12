/*
 * BufferHighlights.rei
 *
 * Per-buffer highlight info
 */
open EditorCoreTypes;

type t;

let initial: t;

let setMatchingPair: (int, Location.t, Location.t, t) => t;
let getMatchingPair: (int, t) => option((Location.t, Location.t));
let clearMatchingPair: (int, t) => t;

let setSearchHighlights: (int, list(Range.t), t) => t;
let clearSearchHighlights: (int, t) => t;

let setDocumentHighlights: (int, list(Range.t), t) => t;
let clearDocumentHighlights: (int, t) => t;

let getHighlightsByLine: (~bufferId: int, ~line: Index.t, t) => list(Range.t);
let getHighlights: (~bufferId: int, t) => list(Index.t);
