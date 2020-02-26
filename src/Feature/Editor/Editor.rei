open EditorCoreTypes;
open Oni_Core;

[@deriving show]
type t = {
  editorId: EditorId.t,
  bufferId: int,
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
};

type scrollbarMetrics = {
  visible: bool,
  thumbSize: int,
  thumbOffset: int,
};

let create: (~bufferId: int=?, unit) => t;

let getId: t => int;
let getTopVisibleLine: (t, EditorMetrics.t) => int;
let getBottomVisibleLine: (t, EditorMetrics.t) => int;
let getLeftVisibleColumn: (t, EditorMetrics.t) => int;
let getLayout: (t, EditorMetrics.t) => EditorLayout.t;
let getPrimaryCursor: t => Location.t;
let getVisibleView: EditorMetrics.t => int; // TODO: Move to EditorMetrics?
let getTotalSizeInPixels: (t, EditorMetrics.t) => int;
let getVerticalScrollbarMetrics: (t, int, EditorMetrics.t) => scrollbarMetrics;
let getHorizontalScrollbarMetrics:
  (t, int, EditorMetrics.t) => scrollbarMetrics;
let pixelPositionToLineColumn:
  (t, EditorMetrics.t, float, float) => (int, int);
let getVimCursors: t => list(Vim.Cursor.t);
