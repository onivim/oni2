open Oni_Core;
open Oni_Core.Types;

type t =
  Actions.editor = {
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
    cursors: list(Vim.Cursor.t),
    selection: VisualRange.t,
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
let getPrimaryCursor: t => Position.t;
let getVisibleView: EditorMetrics.t => int; // TODO: Move to EditorMetrics?
let getTotalSizeInPixels: (t, EditorMetrics.t) => int;
let getVerticalScrollbarMetrics: (t, int, EditorMetrics.t) => scrollbarMetrics;
let getHorizontalScrollbarMetrics:
  (t, int, EditorMetrics.t) => scrollbarMetrics;
let pixelPositionToLineColumn:
  (t, EditorMetrics.t, float, float) => (int, int);
let getVimCursors: t => list(Vim.Cursor.t);
let getLinesAndColumns: (t, EditorMetrics.t) => (int, int);

let reduce: (t, Actions.t, EditorMetrics.t) => t;
