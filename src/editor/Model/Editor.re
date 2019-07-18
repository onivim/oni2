open Oni_Core;
open Oni_Core.Types;

open Actions;

let lastId = ref(0);

type t = Actions.editor;

let create = (~bufferId=0, ()) => {
  let id = lastId^;
  incr(lastId);

  let ret: t = {
    editorId: id,
    bufferId,
    scrollX: 0.,
    scrollY: 0.,
    lastTopLine: 0,
    lastLeftCol: 0,
    minimapMaxColumnWidth: Constants.default.minimapMaxColumn,
    minimapScrollY: 0.,
    maxLineLength: 0,
    viewLines: 0,
    /*
     * We need an initial editor size, otherwise we'll immediately scroll the view
     * if a buffer loads prior to our first render.
     */
    cursorPosition: Position.createFromZeroBasedIndices(0, 0),
    selection: VisualRange.create(),
  };
  ret;
};

type scrollbarMetrics = {
  visible: bool,
  thumbSize: int,
  thumbOffset: int,
};

/* type viewport = { */
/*   pixelX: int, */
/*   pixelY: int, */
/*   pixelWidth: int, */
/*   pixelHeight: int, */
/* }; */

let pixelPositionToLineColumn = (view: t, metrics: EditorMetrics.t, pixelX: float, pixelY: float) => {
  let line = int_of_float((pixelY +. view.scrollY) /. metrics.lineHeight);
  let column = int_of_float((pixelX +. view.scrollX) /. metrics.characterWidth);
  (line, column)
};

let getVisibleView = (metrics: EditorMetrics.t) =>
  int_of_float(float_of_int(metrics.pixelHeight) /. metrics.lineHeight);

let getTotalSizeInPixels = (view: t, metrics: EditorMetrics.t) =>
  int_of_float(float_of_int(view.viewLines) *. metrics.lineHeight);

let getCursorPixelLine = (view: t, metrics: EditorMetrics.t) =>
  float_of_int(Index.toZeroBasedInt(view.cursorPosition.line))
  *. metrics.lineHeight;

let getCursorPixelColumn = (view: t, metrics: EditorMetrics.t) =>
  float_of_int(Index.toZeroBasedInt(view.cursorPosition.character))
  *. metrics.characterWidth;

let getVerticalScrollbarMetrics =
    (view: t, scrollBarHeight: int, metrics: EditorMetrics.t) => {
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
    (view: t, availableWidth: int, metrics: EditorMetrics.t) => {
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

let scrollTo = (view: t, newScrollY, metrics: EditorMetrics.t) => {
  let newScrollY = max(0., newScrollY);
  let availableScroll =
    max(float_of_int(view.viewLines - 1), 0.) *. metrics.lineHeight;
  let newScrollY = min(newScrollY, availableScroll);

  let scrollPercentage =
    newScrollY /. (availableScroll -. float_of_int(metrics.pixelHeight));
  let minimapLineSize =
    Constants.default.minimapCharacterWidth
    + Constants.default.minimapCharacterHeight;
  let linesInMinimap = metrics.pixelHeight / minimapLineSize;
  let availableMinimapScroll =
    max(view.viewLines - linesInMinimap, 0) * minimapLineSize;
  let newMinimapScroll =
    scrollPercentage *. float_of_int(availableMinimapScroll);

  {...view, minimapScrollY: newMinimapScroll, scrollY: newScrollY};
};

let scrollToLine = (view: t, line: int, metrics: EditorMetrics.t) => {
  let scrollAmount = float_of_int(line) *. metrics.lineHeight;
  {...scrollTo(view, scrollAmount, metrics), lastTopLine: line};
};

let scrollToHorizontal = (view: t, newScrollX, metrics: EditorMetrics.t) => {
  let newScrollX = max(0., newScrollX);

  let layout =
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

  let availableScroll =
    max(
      0.,
      float_of_int(view.maxLineLength)
      *. metrics.characterWidth
      -. layout.bufferWidthInPixels,
    );
  let scrollX = min(newScrollX, availableScroll);
  {...view, scrollX};
};

let getLinesAndColumns = (view: t, metrics: EditorMetrics.t) => {
  open EditorLayout;
  let {bufferWidthInCharacters, bufferHeightInCharacters, _} =
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

  (bufferHeightInCharacters, bufferWidthInCharacters);
};

let scrollToColumn = (view: t, column: int, metrics: EditorMetrics.t) => {
  let scrollAmount = float_of_int(column) *. metrics.characterWidth;
  {...scrollToHorizontal(view, scrollAmount, metrics), lastLeftCol: column};
};

let scroll = (view: t, scrollDeltaY, metrics) => {
  let newScrollY = view.scrollY +. scrollDeltaY;
  scrollTo(view, newScrollY, metrics);
};

type cursorLocation =
  | Top
  | Middle
  | Bottom;

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

let moveCursorToPosition = (~moveCursor, view, position, metrics) =>
  switch (position) {
  | Top =>
    let line = getTopVisibleLine(view, metrics);
    moveCursor(~column=0, ~line);
    view;
  | Middle =>
    let topLine = getTopVisibleLine(view, metrics);
    let bottomLine = getBottomVisibleLine(view, metrics);
    moveCursor(~column=0, ~line=(bottomLine + topLine) / 2);
    view;
  | Bottom =>
    let line = getBottomVisibleLine(view, metrics);
    moveCursor(~column=0, ~line);
    view;
  };

let _getMaxLineLength = (buffer: Buffer.t) => {
  let i = ref(0);
  let lines = Buffer.getNumberOfLines(buffer);

  let max = ref(0);

  while (i^ < lines) {
    let line = i^;
    let length = Buffer.getLineLength(buffer, line);

    if (length > max^) {
      max := length;
    };

    incr(i);
  };

  max^;
};

let recalculate = (view: t, buffer: option(Buffer.t)) =>
  switch (buffer) {
  | Some(b) => {
      ...view,
      viewLines: Buffer.getNumberOfLines(b),
      maxLineLength: _getMaxLineLength(b),
    }
  | None => view
  };

let reduce = (view, action, metrics: EditorMetrics.t) =>
  switch (action) {
  | CursorMove(b) => {...view, cursorPosition: b}
  | SelectionChanged(selection) => {...view, selection}
  | RecalculateEditorView(buffer) => recalculate(view, buffer)
  | EditorScroll(scrollY) => scroll(view, scrollY, metrics)
  | EditorScrollToLine(line) => scrollToLine(view, line, metrics)
  | EditorScrollToColumn(column) => scrollToColumn(view, column, metrics)
  | _ => view
  };
