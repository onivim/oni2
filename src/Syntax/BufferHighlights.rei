/*
 * BufferHighlights.rei
 *
 * Per-buffer highlight info
 */
open EditorCoreTypes;

[@deriving show({with_path: false})]
type action =
  | DocumentHighlightsAvailable(int, list(Range.t))
  | DocumentHighlightsCleared(int);

type t;

let initial: t;

let setSearchHighlights: (int, list(Range.t), t) => t;
let clearSearchHighlights: (int, t) => t;

let setDocumentHighlights: (int, list(Range.t), t) => t;
let clearDocumentHighlights: (int, t) => t;

let getHighlightsByLine: (~bufferId: int, ~line: Index.t, t) => list(Range.t);
let getHighlights: (~bufferId: int, t) => list(Index.t);
