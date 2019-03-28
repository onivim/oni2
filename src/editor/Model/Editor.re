open Oni_Core;
open Oni_Core.Types;

open Actions;

[@deriving show]
type t = {
  id: int,
  scrollX: float,
  scrollY: float,
  minimapScrollY: float,
  /*
   * The maximum line visible in the view.
   * TODO: This will be dependent on line-wrap settings.
   */
  maxLineLength: int,
  viewLines: int,
  size: EditorSize.t,
  cursorPosition: Position.t,
  lineHeight: float,
  characterWidth: float,
  selection: VisualRange.t,
};

let create = () => {
  let ret: t = {
    id: 0,
    scrollX: 0.,
    scrollY: 0.,
    minimapScrollY: 0.,
    maxLineLength: 0,
    viewLines: 0,
    /*
     * We need an initial editor size, otherwise we'll immediately scroll the view
     * if a buffer loads prior to our first render.
     */
    size: EditorSize.create(~pixelWidth=1000, ~pixelHeight=1000, ()),
    cursorPosition: Position.createFromZeroBasedIndices(0, 0),
    lineHeight: 1.,
    characterWidth: 1.,
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

let getVisibleView = (view: t) =>
  int_of_float(float_of_int(view.size.pixelWidth) /. view.lineHeight);

let getTotalSizeInPixels = (view: t) =>
  int_of_float(float_of_int(view.viewLines) *. view.lineHeight);

let getCursorPixelLine = (view: t) =>
  float_of_int(Index.toZeroBasedInt(view.cursorPosition.line))
  *. view.lineHeight;

let getCursorPixelColumn = (view: t) =>
  float_of_int(Index.toZeroBasedInt(view.cursorPosition.character))
  *. view.characterWidth;

let getVerticalScrollbarMetrics = (view: t, scrollBarHeight: int) => {
  let totalViewSizeInPixels =
    float_of_int(getTotalSizeInPixels(view) + view.size.pixelHeight);
  let thumbPercentage =
    float_of_int(view.size.pixelHeight) /. totalViewSizeInPixels;
  let thumbSize =
    int_of_float(thumbPercentage *. float_of_int(scrollBarHeight));

  let topF = view.scrollY /. totalViewSizeInPixels;
  let thumbOffset = int_of_float(topF *. float_of_int(scrollBarHeight));

  {thumbSize, thumbOffset, visible: true};
};

let getHorizontalScrollbarMetrics = (view: t, availableWidth: int) => {
  let totalViewWidthInPixels =
    float_of_int(view.maxLineLength) *. view.characterWidth;
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

let scrollTo = (view: t, newScrollY) => {
  let newScrollY = max(0., newScrollY);
  let availableScroll =
    max(float_of_int(view.viewLines - 1), 0.) *. view.lineHeight;
  let newScrollY = min(newScrollY, availableScroll);

  let scrollPercentage =
    newScrollY /. (availableScroll -. float_of_int(view.size.pixelHeight));
  let minimapLineSize =
    Constants.default.minimapCharacterWidth
    + Constants.default.minimapCharacterHeight;
  let linesInMinimap = view.size.pixelHeight / minimapLineSize;
  let availableMinimapScroll =
    max(view.viewLines - linesInMinimap, 0) * minimapLineSize;
  let newMinimapScroll =
    scrollPercentage *. float_of_int(availableMinimapScroll);

  {...view, minimapScrollY: newMinimapScroll, scrollY: newScrollY};
};

let scrollToHorizontal = (view: t, newScrollX) => {
  let newScrollX = max(0., newScrollX);

  let layout =
    EditorLayout.getLayout(
      ~pixelWidth=float_of_int(view.size.pixelWidth),
      ~pixelHeight=float_of_int(view.size.pixelHeight),
      ~isMinimapShown=true,
      ~characterWidth=view.characterWidth,
      ~characterHeight=view.lineHeight,
      ~bufferLineCount=view.viewLines,
      (),
    );

  let availableScroll =
    max(
      0.,
      float_of_int(view.maxLineLength)
      *. view.characterWidth
      -. layout.bufferWidthInPixels,
    );
  let scrollX = min(newScrollX, availableScroll);
  {...view, scrollX};
};

let scroll = (view: t, scrollDeltaY) => {
  let newScrollY = view.scrollY +. scrollDeltaY;
  scrollTo(view, newScrollY);
};

/* Scroll so that the cursor is at the TOP of the view */
let scrollToCursorTop = (view: t) => {
  let scrollPosition = getCursorPixelLine(view);
  scrollTo(view, scrollPosition);
};

/* Scroll so that the cursor is at the BOTTOM of the view */
let scrollToCursorBottom = (view: t) => {
  let cursorPixelPosition = getCursorPixelLine(view);
  let scrollPosition =
    cursorPixelPosition
    -. (float_of_int(view.size.pixelHeight) -. view.lineHeight);
  scrollTo(view, scrollPosition);
};

/* Scroll so that the cursor is at the LEFT of the view */
let scrollToCursorLeft = (view: t) => {
  let cursorPixelColumn = getCursorPixelColumn(view);

  let scrollPosition = cursorPixelColumn;
  scrollToHorizontal(view, scrollPosition);
};

/* Scroll so that the cursor is at the RIGHT of the view */
let scrollToCursorRight = (view: t, availableWidth) => {
  let cursorPixelColumn = getCursorPixelColumn(view);

  let scrollPosition =
    cursorPixelColumn -. availableWidth +. view.characterWidth;
  scrollToHorizontal(view, scrollPosition);
};

let scrollToCursor = (view: t) => {
  let scrollPosition =
    getCursorPixelLine(view)
    -. float_of_int(view.size.pixelHeight / 2)
    +. view.lineHeight
    /. 2.;

  scrollTo(view, scrollPosition);
};

let snapToCursorPosition = (view: t) => {
  let cursorPixelPositionY = getCursorPixelLine(view);
  let scrollY = view.scrollY;

  let view =
    if (cursorPixelPositionY < scrollY) {
      scrollToCursorTop(view);
    } else if (cursorPixelPositionY > scrollY
               +. float_of_int(view.size.pixelHeight)
               -. view.lineHeight) {
      scrollToCursorBottom(view);
    } else {
      view;
    };

  let layout =
    EditorLayout.getLayout(
      ~pixelWidth=float_of_int(view.size.pixelWidth),
      ~pixelHeight=float_of_int(view.size.pixelHeight),
      ~isMinimapShown=true,
      ~characterWidth=view.characterWidth,
      ~characterHeight=view.lineHeight,
      ~bufferLineCount=view.viewLines,
      (),
    );

  let cursorPixelPositionX = getCursorPixelColumn(view);
  let scrollX = view.scrollX;

  let availableWidth = layout.bufferWidthInPixels;

  let view =
    if (cursorPixelPositionX < scrollX) {
      scrollToCursorLeft(view);
    } else if (cursorPixelPositionX >= scrollX
               +. layout.bufferWidthInPixels
               -. view.characterWidth) {
      scrollToCursorRight(view, availableWidth);
    } else {
      view;
    };

  view;
};

type cursorLocation =
  | Top
  | Middle
  | Bottom;

let getTopVisibleLine = view =>
  int_of_float(view.scrollY /. view.lineHeight) + 1;

let getBottomVisibleLine = view => {
  let absoluteBottomLine =
    int_of_float(
      (view.scrollY +. float_of_int(view.size.pixelHeight)) /. view.lineHeight,
    );
  absoluteBottomLine > view.viewLines ? view.viewLines : absoluteBottomLine;
};

let moveCursorToPosition = (~moveCursor, view, position) =>
  switch (position) {
  | Top =>
    let line = getTopVisibleLine(view);
    moveCursor(~column=0, ~line);
    view;
  | Middle =>
    let topLine = getTopVisibleLine(view);
    let bottomLine = getBottomVisibleLine(view);
    moveCursor(~column=0, ~line=(bottomLine + topLine) / 2);
    view;
  | Bottom =>
    let line = getBottomVisibleLine(view);
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

let reduce = (view, action, buffer) =>
  switch (action) {
  | CursorMove(b) => snapToCursorPosition({...view, cursorPosition: b})
  | SetEditorSize(size) => {...view, size}
  | RecalculateEditorView => recalculate(view, buffer)
  | EditorScroll(scrollY) => scroll(view, scrollY)
  | EditorScrollToCursorTop => scrollToCursorTop(view)
  | EditorScrollToCursorBottom => scrollToCursorBottom(view)
  | EditorScrollToCursorCentered => scrollToCursor(view)
  | EditorMoveCursorToTop(moveCursor) =>
    moveCursorToPosition(~moveCursor, view, Top)
  | EditorMoveCursorToMiddle(moveCursor) =>
    moveCursorToPosition(~moveCursor, view, Middle)
  | SetEditorFont({measuredHeight, measuredWidth, _}) => {
      ...view,
      lineHeight: measuredHeight,
      characterWidth: measuredWidth,
    }
  | EditorMoveCursorToBottom(moveCursor) =>
    moveCursorToPosition(~moveCursor, view, Bottom)
  | _ => view
  };
