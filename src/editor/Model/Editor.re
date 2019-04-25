open Oni_Core;
open Oni_Core.Types;

open Actions;

let lastId = ref(0);

type t = Actions.editor;

let create = (~bufferId=0, ()) => {
  let id = lastId^;
  incr(lastId);

  let ret: t = {
    id,
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

let scroll = (view: t, scrollDeltaY, metrics) => {
  let newScrollY = view.scrollY +. scrollDeltaY;
  scrollTo(view, newScrollY, metrics);
};

/* Scroll so that the cursor is at the TOP of the view */
let scrollToCursorTop = (view: t, metrics) => {
  let scrollPosition = getCursorPixelLine(view, metrics);
  scrollTo(view, scrollPosition, metrics);
};

/* Scroll so that the cursor is at the BOTTOM of the view */
let scrollToCursorBottom = (view: t, metrics: EditorMetrics.t) => {
  let cursorPixelPosition = getCursorPixelLine(view, metrics);
  let scrollPosition =
    cursorPixelPosition
    -. (float_of_int(metrics.pixelHeight) -. metrics.lineHeight);
  scrollTo(view, scrollPosition, metrics);
};

/* Scroll so that the cursor is at the LEFT of the view */
let scrollToCursorLeft = (view: t, metrics) => {
  let cursorPixelColumn = getCursorPixelColumn(view, metrics);

  let scrollPosition = cursorPixelColumn;
  scrollToHorizontal(view, scrollPosition, metrics);
};

/* Scroll so that the cursor is at the RIGHT of the view */
let scrollToCursorRight = (view: t, availableWidth, metrics) => {
  let cursorPixelColumn = getCursorPixelColumn(view, metrics);

  let scrollPosition =
    cursorPixelColumn -. availableWidth +. metrics.characterWidth;
  scrollToHorizontal(view, scrollPosition, metrics);
};

let scrollToCursor = (view: t, metrics: EditorMetrics.t) => {
  let scrollPosition =
    getCursorPixelLine(view, metrics)
    -. float_of_int(metrics.pixelHeight / 2)
    +. metrics.lineHeight
    /. 2.;

  scrollTo(view, scrollPosition, metrics);
};

let snapToCursorPosition = (view: t, metrics: EditorMetrics.t) => {
  let cursorPixelPositionY = getCursorPixelLine(view, metrics);
  let scrollY = view.scrollY;

  let view =
    if (cursorPixelPositionY < scrollY) {
      scrollToCursorTop(view, metrics);
    } else if (cursorPixelPositionY > scrollY
               +. float_of_int(metrics.pixelHeight)
               -. metrics.lineHeight) {
      scrollToCursorBottom(view, metrics);
    } else {
      view;
    };

  let layout =
    EditorLayout.getLayout(
      ~pixelWidth=float_of_int(metrics.pixelWidth),
      ~pixelHeight=float_of_int(metrics.pixelHeight),
      ~isMinimapShown=true,
      ~characterWidth=metrics.characterWidth,
      ~characterHeight=metrics.lineHeight,
      ~bufferLineCount=view.viewLines,
      (),
    );

  let cursorPixelPositionX = getCursorPixelColumn(view, metrics);
  let scrollX = view.scrollX;

  let availableWidth = layout.bufferWidthInPixels;

  let view =
    if (cursorPixelPositionX < scrollX) {
      scrollToCursorLeft(view, metrics);
    } else if (cursorPixelPositionX >= scrollX
               +. layout.bufferWidthInPixels
               -. metrics.characterWidth) {
      scrollToCursorRight(view, availableWidth, metrics);
    } else {
      view;
    };

  view;
};

type cursorLocation =
  | Top
  | Middle
  | Bottom;

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
  | CursorMove(b) =>
    snapToCursorPosition({...view, cursorPosition: b}, metrics)
  | SelectionChanged(selection) => {...view, selection}
  | RecalculateEditorView(buffer) => recalculate(view, buffer)
  | EditorScroll(scrollY) => scroll(view, scrollY, metrics)
  | EditorScrollToCursorTop => scrollToCursorTop(view, metrics)
  | EditorScrollToCursorBottom => scrollToCursorBottom(view, metrics)
  | EditorScrollToCursorCentered => scrollToCursor(view, metrics)
  | EditorMoveCursorToTop(moveCursor) =>
    moveCursorToPosition(~moveCursor, view, Top, metrics)
  | EditorMoveCursorToMiddle(moveCursor) =>
    moveCursorToPosition(~moveCursor, view, Middle, metrics)
  | EditorMoveCursorToBottom(moveCursor) =>
    moveCursorToPosition(~moveCursor, view, Bottom, metrics)
  | _ => view
  };
