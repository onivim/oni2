open Actions;
open Types;

type t = {
  id: int,
  scrollY: int,
  minimapScrollY: int,
  viewLines: int,
  size: EditorSize.t,
  cursorPosition: BufferPosition.t,
  lineHeight: int,
};

let create = () => {
  let ret: t = {
    id: 0,
    scrollY: 0,
    minimapScrollY: 0,
    viewLines: 0,
    size: EditorSize.create(~pixelWidth=0, ~pixelHeight=0, ()),
    cursorPosition: BufferPosition.createFromZeroBasedIndices(0, 0),
    lineHeight: 1,
  };
  ret;
};

type scrollbarMetrics = {
  thumbSize: int,
  thumbOffset: int,
};

type viewport = {
  pixelX: int,
  pixelY: int,
  pixelWidth: int,
  pixelHeight: int,
};

let getVisibleLines = (view: t) => view.size.pixelWidth / view.lineHeight;

let getTotalSizeInPixels = (view: t) => view.viewLines * view.lineHeight;

let getCursorPixelLine = (view: t) =>
  Index.toZeroBasedInt(view.cursorPosition.line) * view.lineHeight;

let getScrollbarMetrics = (view: t, scrollBarHeight: int) => {
  let totalViewSizeInPixels =
    float_of_int(getTotalSizeInPixels(view) + view.size.pixelHeight);
  let thumbPercentage =
    float_of_int(view.size.pixelHeight) /. totalViewSizeInPixels;
  let thumbSize =
    int_of_float(thumbPercentage *. float_of_int(scrollBarHeight));

  let topF = float_of_int(view.scrollY) /. totalViewSizeInPixels;
  let thumbOffset = int_of_float(topF *. float_of_int(scrollBarHeight));

  {thumbSize, thumbOffset};
};

let scrollTo = (view: t, newScrollY) => {
  let newScrollY = max(0, newScrollY);
  let availableScroll = max(view.viewLines - 1, 0) * view.lineHeight;
  let newScrollY = min(newScrollY, availableScroll);

  let scrollPercentage =
    float_of_int(newScrollY)
    /. float_of_int(availableScroll - view.size.pixelHeight);
  let minimapLineSize =
    Constants.default.minimapCharacterWidth
    + Constants.default.minimapCharacterHeight;
  let linesInMinimap = view.size.pixelHeight / minimapLineSize;
  let availableMinimapScroll =
    max(view.viewLines - linesInMinimap, 0) * minimapLineSize;
  let newMinimapScroll =
    int_of_float(scrollPercentage *. float_of_int(availableMinimapScroll));

  {...view, minimapScrollY: newMinimapScroll, scrollY: newScrollY};
};

let scroll = (view: t, scrollDeltaY) => {
  let newScrollY = view.scrollY + scrollDeltaY;
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
    cursorPixelPosition - (view.size.pixelHeight - view.lineHeight * 1);
  scrollTo(view, scrollPosition);
};

let scrollToCursor = (view: t) => {
  let scrollPosition =
    getCursorPixelLine(view) - view.size.pixelHeight / 2 + view.lineHeight / 2;

  scrollTo(view, scrollPosition);
};

let snapToCursorPosition = (view: t) => {
  let cursorPixelPosition = getCursorPixelLine(view);
  let scrollY = view.scrollY;

  if (cursorPixelPosition < scrollY) {
    scrollToCursorTop(view);
  } else if (cursorPixelPosition > scrollY
             + view.size.pixelHeight
             - view.lineHeight) {
    scrollToCursorBottom(view);
  } else {
    view;
  };
};

type cursorLocation =
  | Top
  | Middle
  | Bottom;

let getTopVisibleLine = view => view.scrollY / view.lineHeight + 1;

let getBottomVisibleLine = view => {
  let absoluteBottomLine =
    (view.scrollY + view.size.pixelHeight) / view.lineHeight;
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

let recalculate = (view: t, buffer: option(Buffer.t)) =>
  switch (buffer) {
  | Some(b) => {...view, viewLines: Array.length(b.lines)}
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
  | SetEditorFont({measuredHeight, _}) => {
      ...view,
      lineHeight: measuredHeight,
    }
  | EditorMoveCursorToBottom(moveCursor) =>
    moveCursorToPosition(~moveCursor, view, Bottom)
  | _ => view
  };
