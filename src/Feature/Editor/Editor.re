open EditorCoreTypes;
open Oni_Core;

let lastId = ref(0);

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

let create = (~bufferId=0, ()) => {
  let id = lastId^;
  incr(lastId);

  {
    editorId: id,
    bufferId,
    scrollX: 0.,
    scrollY: 0.,
    minimapMaxColumnWidth: Constants.default.minimapMaxColumn,
    minimapScrollY: 0.,
    maxLineLength: 0,
    viewLines: 0,
    /*
     * We need an initial editor size, otherwise we'll immediately scroll the view
     * if a buffer loads prior to our first render.
     */
    cursors: [Vim.Cursor.create(~line=Index.zero, ~column=Index.zero)],
    selection:
      VisualRange.create(
        ~mode=Vim.Types.None,
        Range.{
          start: Location.{line: Index.zero, column: Index.zero},
          stop: Location.{line: Index.zero, column: Index.zero},
        },
      ),
  };
};

type scrollbarMetrics = {
  visible: bool,
  thumbSize: int,
  thumbOffset: int,
};

let getVimCursors = model => model.cursors;

let getPrimaryCursor = model =>
  switch (model.cursors) {
  | [cursor, ..._] => (cursor :> Location.t)
  | [] => Location.{line: Index.zero, column: Index.zero}
  };

let getId = model => model.editorId;

let pixelPositionToLineColumn =
    (view, metrics: EditorMetrics.t, pixelX, pixelY) => {
  let line = int_of_float((pixelY +. view.scrollY) /. metrics.lineHeight);
  let column =
    int_of_float((pixelX +. view.scrollX) /. metrics.characterWidth);

  (line, column);
};

let getVisibleView = (metrics: EditorMetrics.t) =>
  int_of_float(float_of_int(metrics.pixelHeight) /. metrics.lineHeight);

let getTotalSizeInPixels = (view, metrics: EditorMetrics.t) =>
  int_of_float(float_of_int(view.viewLines) *. metrics.lineHeight);

let getVerticalScrollbarMetrics =
    (view, scrollBarHeight, metrics: EditorMetrics.t) => {
  let totalViewSizeInPixels =
    float_of_int(getTotalSizeInPixels(view, metrics) + metrics.pixelHeight);
  let thumbPercentage =
    float_of_int(metrics.pixelHeight) /. totalViewSizeInPixels;
  let thumbSize =
    int_of_float(thumbPercentage *. float_of_int(scrollBarHeight));

  let topF = view.scrollY /. totalViewSizeInPixels;
  let thumbOffset = int_of_float(topF *. float_of_int(scrollBarHeight));

  {thumbSize, thumbOffset, visible: true};
};

let getHorizontalScrollbarMetrics =
    (view, availableWidth, metrics: EditorMetrics.t) => {
  let totalViewWidthInPixels =
    float_of_int(view.maxLineLength) *. metrics.characterWidth;
  let availableWidthF = float_of_int(availableWidth);

  totalViewWidthInPixels <= availableWidthF
    ? {visible: false, thumbSize: 0, thumbOffset: 0}
    : {
      let thumbPercentage = availableWidthF /. totalViewWidthInPixels;
      let thumbSize = int_of_float(thumbPercentage *. availableWidthF);

      let topF = view.scrollX /. totalViewWidthInPixels;
      let thumbOffset = int_of_float(topF *. availableWidthF);

      {thumbSize, thumbOffset, visible: true};
    };
};

let getLayout = (view, metrics: EditorMetrics.t) => {
  let layout: EditorLayout.t =
    EditorLayout.getLayout(
      ~maxMinimapCharacters=view.minimapMaxColumnWidth,
      ~pixelWidth=float_of_int(metrics.pixelWidth),
      ~pixelHeight=float_of_int(metrics.pixelHeight),
      ~isMinimapShown=true,
      ~characterWidth=metrics.characterWidth,
      ~characterHeight=metrics.lineHeight,
      ~bufferLineCount=view.viewLines,
      (),
    );

  layout;
};

let getLeftVisibleColumn = (view, metrics: EditorMetrics.t) => {
  int_of_float(view.scrollX /. metrics.characterWidth);
};

let getTopVisibleLine = (view, metrics: EditorMetrics.t) =>
  int_of_float(view.scrollY /. metrics.lineHeight) + 1;

let getBottomVisibleLine = (view, metrics: EditorMetrics.t) => {
  let absoluteBottomLine =
    int_of_float(
      (view.scrollY +. float_of_int(metrics.pixelHeight))
      /. metrics.lineHeight,
    );

  absoluteBottomLine > view.viewLines ? view.viewLines : absoluteBottomLine;
};
