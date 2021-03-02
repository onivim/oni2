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

let moveMarkers:
  (~newBuffer: Oni_Core.Buffer.t, ~markerUpdate: Oni_Core.MarkerUpdate.t, t) =>
  t;
