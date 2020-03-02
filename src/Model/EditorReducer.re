open Oni_Core;
open Feature_Editor;
open Editor;

module Internal = {
  let getMaxLineLength = (buffer: Buffer.t) => {
    let i = ref(0);
    let lines = Buffer.getNumberOfLines(buffer);

    let max = ref(0);

    while (i^ < lines) {
      let line = i^;
      // TODO: This is approximate, beacuse the length in bytes isn't actually
      // the max length. But the length in bytes is quicker to calculate.
      let length = buffer |> Buffer.getLine(line) |> BufferLine.lengthInBytes;

      if (length > max^) {
        max := length;
      };

      incr(i);
    };

    max^;
  };
};

let scrollTo = (view, newScrollY, metrics: EditorMetrics.t) => {
  let newScrollY = max(0., newScrollY);
  let availableScroll =
    max(float_of_int(view.viewLines - 1), 0.) *. metrics.lineHeight;
  let newScrollY = min(newScrollY, availableScroll);

  let scrollPercentage =
    newScrollY /. (availableScroll -. float_of_int(metrics.pixelHeight));
  let minimapLineSize =
    Constants.minimapCharacterWidth + Constants.minimapCharacterHeight;
  let linesInMinimap = metrics.pixelHeight / minimapLineSize;
  let availableMinimapScroll =
    max(view.viewLines - linesInMinimap, 0) * minimapLineSize;
  let newMinimapScroll =
    scrollPercentage *. float_of_int(availableMinimapScroll);

  {...view, minimapScrollY: newMinimapScroll, scrollY: newScrollY};
};

let scrollToLine = (view, line, metrics: EditorMetrics.t) => {
  let scrollAmount = float_of_int(line) *. metrics.lineHeight;
  scrollTo(view, scrollAmount, metrics);
};

let scrollToHorizontal = (view, newScrollX, metrics: EditorMetrics.t) => {
  let newScrollX = max(0., newScrollX);

  let availableScroll =
    max(
      0.,
      float_of_int(view.maxLineLength)
      *. metrics.characterWidth
      -. float(metrics.pixelWidth),
    );
  let scrollX = min(newScrollX, availableScroll);

  {...view, scrollX};
};

let scrollToColumn = (view, column, metrics: EditorMetrics.t) => {
  let scrollAmount = float_of_int(column) *. metrics.characterWidth;
  scrollToHorizontal(view, scrollAmount, metrics);
};

let scroll = (view, scrollDeltaY, metrics) => {
  let newScrollY = view.scrollY +. scrollDeltaY;
  scrollTo(view, newScrollY, metrics);
};

let recalculate = (view, maybeBuffer) =>
  switch (maybeBuffer) {
  | Some(buffer) => {
      ...view,
      viewLines: Buffer.getNumberOfLines(buffer),
      maxLineLength: Internal.getMaxLineLength(buffer),
    }
  | None => view
  };

let reduce = (view, action, metrics: EditorMetrics.t) =>
  switch ((action: Actions.t)) {
  | SelectionChanged(selection) => {...view, selection}
  | RecalculateEditorView(buffer) => recalculate(view, buffer)
  | EditorCursorMove(id, cursors) when EditorId.equals(view.editorId, id) => {
      ...view,
      cursors,
    }
  | EditorSetScroll(id, scrollY) when EditorId.equals(view.editorId, id) =>
    scrollTo(view, scrollY, metrics)
  | EditorScroll(id, scrollDeltaY) when EditorId.equals(view.editorId, id) =>
    scroll(view, scrollDeltaY, metrics)
  | EditorScrollToLine(id, line) when EditorId.equals(view.editorId, id) =>
    scrollToLine(view, line, metrics)
  | EditorScrollToColumn(id, column) when EditorId.equals(view.editorId, id) =>
    scrollToColumn(view, column, metrics)
  | _ => view
  };
