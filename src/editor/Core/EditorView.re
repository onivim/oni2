open Actions;
open Types;

type t = {
  id: int,
  scrollY: int,
  viewLines: int,
  size: EditorSize.t,
};

let create = (~scrollY=0, ()) => {
  let ret: t = {
    id: 0,
    scrollY,
    viewLines: 0,
    size: EditorSize.create(~pixelWidth=0, ~pixelHeight=0, ()),
  };
  ret;
};

type scrollbarMetrics = {
  thumbSize: int,
  thumbOffset: int,
};

let getVisibleLines = (view: t, lineHeight: int) => {
  view.size.pixelWidth / lineHeight;
};

let getTotalSizeInPixels = (view: t, lineHeight: int) => {
  /* let visibleLines = getVisibleLines(view, lineHeight); */
  view.viewLines * lineHeight;
};

let getScrollbarMetrics = (view: t, scrollBarHeight: int, lineHeight: int) => {
  /* let lineHeight = 1; */
  /* let linesInView = getSizeOfViewInLines(view, lineHeight); */
  let totalViewSizeInPixels =
    float_of_int(getTotalSizeInPixels(view, lineHeight));
  let thumbPercentage =
    float_of_int(view.size.pixelHeight) /. totalViewSizeInPixels;
  let thumbSize =
    int_of_float(thumbPercentage *. float_of_int(scrollBarHeight));

  let topF = float_of_int(view.scrollY) /. totalViewSizeInPixels;
  let thumbOffset = int_of_float(topF *. float_of_int(scrollBarHeight));

  {thumbSize, thumbOffset};
};

let scroll = (view: t, scrollDeltaY, measuredFontHeight) => {
  let newScrollY = view.scrollY + scrollDeltaY;
  let newScrollY = max(0, newScrollY);

  let availableScroll = max(view.viewLines - 1, 0) * measuredFontHeight;
  let newScrollY = min(newScrollY, availableScroll);

  {...view, scrollY: newScrollY};
};

let recalculate = (view: t, buffer: Buffer.t) => {
  {...view, viewLines: Array.length(buffer.lines)};
};

let reduce = (view, action, buffer, fontMetrics: EditorFont.t) => {
  switch (action) {
  | SetEditorSize(size) => {...view, size}
  | RecalculateEditorView => recalculate(view, buffer)
  | EditorScroll(scrollY) =>
    scroll(view, scrollY, fontMetrics.measuredHeight)
  | _ => view
  };
};
