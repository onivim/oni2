open Actions;
open Types;

type t = {
  id: int,
  scrollY: int,
  viewLines: int,
};

let create = (~scrollY=0, ()) => {
  let ret: t = {id: 0, scrollY, viewLines: 0};
  ret;
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
  | RecalculateEditorView => recalculate(view, buffer)
  | EditorScroll(scrollY) =>
    scroll(view, scrollY, fontMetrics.measuredHeight)
  | _ => view
  };
};
