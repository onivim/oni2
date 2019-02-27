open Actions;
open Types;

type t = {
  id: int,
  scrollY: int,
  minimapScrollY: int,
  viewLines: int,
  size: EditorSize.t,
  cursorPosition: BufferPosition.t,
};

let create = () => {
  let ret: t = {
    id: 0,
    scrollY: 0,
    minimapScrollY: 0,
    viewLines: 0,
    size: EditorSize.create(~pixelWidth=0, ~pixelHeight=0, ()),
    cursorPosition: BufferPosition.createFromZeroBasedIndices(0, 0),
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

let getVisibleLines = (view: t, lineHeight: int) =>
  view.size.pixelWidth / lineHeight;

let getTotalSizeInPixels = (view: t, lineHeight: int) =>
  view.viewLines * lineHeight;

let getCursorPixelLine = (view: t, lineHeight: int) =>
  Index.toZeroBasedInt(view.cursorPosition.line) * lineHeight;

let getScrollbarMetrics = (view: t, scrollBarHeight: int, lineHeight: int) => {
  let totalViewSizeInPixels =
    float_of_int(
      getTotalSizeInPixels(view, lineHeight) + view.size.pixelHeight,
    );
  let thumbPercentage =
    float_of_int(view.size.pixelHeight) /. totalViewSizeInPixels;
  let thumbSize =
    int_of_float(thumbPercentage *. float_of_int(scrollBarHeight));

  let topF = float_of_int(view.scrollY) /. totalViewSizeInPixels;
  let thumbOffset = int_of_float(topF *. float_of_int(scrollBarHeight));

  {thumbSize, thumbOffset};
};

let scrollTo = (view: t, newScrollY, measuredFontHeight) => {
  let newScrollY = max(0, newScrollY);
  let availableScroll = max(view.viewLines - 1, 0) * measuredFontHeight;
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

let scroll = (view: t, scrollDeltaY, measuredFontHeight) => {
  let newScrollY = view.scrollY + scrollDeltaY;
  scrollTo(view, newScrollY, measuredFontHeight);
};

/* Scroll so that the cursor is at the TOP of the view */
let scrollToCursorTop = (view: t, lineHeight) => {
  let scrollPosition = getCursorPixelLine(view, lineHeight);
  scrollTo(view, scrollPosition, lineHeight);
};

/* Scroll so that the cursor is at the BOTTOM of the view */
let scrollToCursorBottom = (view: t, lineHeight) => {
  let cursorPixelPosition = getCursorPixelLine(view, lineHeight);
  let scrollPosition =
    cursorPixelPosition - (view.size.pixelHeight - lineHeight * 1);
  scrollTo(view, scrollPosition, lineHeight);
};

let scrollToCursor = (view: t, lineHeight) => {
  let scrollPosition =
    getCursorPixelLine(view, lineHeight)
    - view.size.pixelHeight
    / 2
    + lineHeight
    / 2;

  scrollTo(view, scrollPosition, lineHeight);
};

let snapToCursorPosition = (view: t, lineHeight: int) => {
  let cursorPixelPosition = getCursorPixelLine(view, lineHeight);
  let scrollY = view.scrollY;

  if (cursorPixelPosition < scrollY) {
    scrollToCursorTop(view, lineHeight);
  } else if (cursorPixelPosition > scrollY + view.size.pixelHeight - lineHeight) {
    scrollToCursorBottom(view, lineHeight);
  } else {
    view;
  };
};

let recalculate = (view: t, buffer: option(Buffer.t)) =>
  switch (buffer) {
  | Some(b) => {...view, viewLines: Array.length(b.lines)}
  | None => view
  };

let reduce = (view, action, buffer, fontMetrics: EditorFont.t) =>
  switch (action) {
  | CursorMove(b) =>
    snapToCursorPosition(
      {...view, cursorPosition: b},
      fontMetrics.measuredHeight,
    )
  | SetEditorSize(size) => {...view, size}
  | RecalculateEditorView => recalculate(view, buffer)
  | EditorScroll(scrollY) =>
    scroll(view, scrollY, fontMetrics.measuredHeight)
  | EditorScrollToCursorTop =>
    scrollToCursorTop(view, fontMetrics.measuredHeight)
  | EditorScrollToCursorBottom =>
    scrollToCursorBottom(view, fontMetrics.measuredHeight)
  | EditorScrollToCursorCentered =>
    scrollToCursor(view, fontMetrics.measuredHeight)
  | _ => view
  };
