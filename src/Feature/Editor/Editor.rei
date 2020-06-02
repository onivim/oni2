open EditorCoreTypes;
open Oni_Core;

[@deriving show]
type t = {
  buffer: EditorBuffer.t,
  editorId: EditorId.t,
  scrollX: float,
  scrollY: float,
  minimapMaxColumnWidth: int,
  minimapScrollY: float,
  /*
   * The maximum line visible in the view.
   * TODO: This will be dependent on line-wrap settings.
   */
  maxLineLength: int,
  viewLines: int,
  cursors: [@opaque] list(Vim.Cursor.t),
  selection: [@opaque] VisualRange.t,
  font: [@opaque] Service_Font.font,
  pixelWidth: int,
  pixelHeight: int,
};

type scrollbarMetrics = {
  visible: bool,
  thumbSize: int,
  thumbOffset: int,
};

let create: (~font: Service_Font.font, ~buffer: EditorBuffer.t, unit) => t;

let getId: t => int;
let getBufferId: t => int;
let getTopVisibleLine: t => int;
let getBottomVisibleLine: t => int;
let getLeftVisibleColumn: t => int;
let getLayout: t => EditorLayout.t;
let getCharacterUnderCursor: (~buffer: Buffer.t, t) => option(Uchar.t);
let getPrimaryCursor: (~buffer: Buffer.t, t) => Location.t;
let getVisibleView: t => int;
let getTotalHeightInPixels: t => int;
let getVerticalScrollbarMetrics: (t, int) => scrollbarMetrics;
let getHorizontalScrollbarMetrics: (t, int) => scrollbarMetrics;
let pixelPositionToBufferLineByte:
  (~buffer: Buffer.t, ~pixelX: float, ~pixelY: float, t) => (int, int);
let getVimCursors: t => list(Vim.Cursor.t);

let scrollToColumn: (~column: int, t) => t;
let scrollToPixelX: (~pixelX: float, t) => t;

let scrollToLine: (~line: int, t) => t;
let scrollToPixelY: (~pixelY: float, t) => t;
let scrollDeltaPixelY: (~pixelY: float, t) => t;

let getCharacterWidth: t => float;
let getLineHeight: t => float;

// PROJECTION

// Convert (or unconvent) a (line, column) to a different coordinate space

// [project] - given a zero-based [line] and [column], and a pixel space
// defined by [pixelWidth] and [pixelHeight], return the [(pixelX, pixelY)]
// corresponding to the top-left of the line and column.
let project: (~line: int, ~column: int, ~pixelWidth: int, ~pixelHeight: int, t)
=> (float, float)

// [projectLine] - like [project], but ignoring the [column]/[width]
let projectLine: (~line: int, ~pixelHeight: int, t) => float;

// [unproject] - given a pixel space defined by [pixelWidth] and [pixelHeight],
// map the [pixelX] and [pixelY] of that space to a pixel position on the editor
// surface - [(surfacePixelX, surfacePixelY)]

let unprojectToPixel: (
~pixelX: float,
~pixelY: float, 
~pixelWidth: int,
~pixelHeight: int,
t
) => (float, float);


let setFont: (~font: Service_Font.font, t) => t;
let setSize: (~pixelWidth: int, ~pixelHeight: int, t) => t;

let updateBuffer: (~buffer: EditorBuffer.t, t) => t;
