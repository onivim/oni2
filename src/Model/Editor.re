open Oni_Core;

let lastId = ref(0);

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
    cursors: [Vim.Cursor.create(~line=1, ~column=0, ())],
    selection: VisualRange.create(),
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
  | [hd, ..._] =>
    let line = Index.ofInt1(hd.line);
    let character = Index.ofInt0(hd.column);
    Position.create(line, character);
  | [] => Position.ofInt0(0, 0)
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

let scrollTo = (view, newScrollY, metrics: EditorMetrics.t) => {
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

let scrollToLine = (view, line, metrics: EditorMetrics.t) => {
  let scrollAmount = float_of_int(line) *. metrics.lineHeight;
  scrollTo(view, scrollAmount, metrics);
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

let scrollToHorizontal = (view, newScrollX, metrics: EditorMetrics.t) => {
  let newScrollX = max(0., newScrollX);

  let layout = getLayout(view, metrics);

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

let getLinesAndColumns = (view, metrics: EditorMetrics.t) => {
  let layout = getLayout(view, metrics);

  (layout.bufferHeightInCharacters, layout.bufferWidthInCharacters);
};

let scrollToColumn = (view, column, metrics: EditorMetrics.t) => {
  let scrollAmount = float_of_int(column) *. metrics.characterWidth;
  scrollToHorizontal(view, scrollAmount, metrics);
};

let scroll = (view, scrollDeltaY, metrics) => {
  let newScrollY = view.scrollY +. scrollDeltaY;
  scrollTo(view, newScrollY, metrics);
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

let recalculate = (view, maybeBuffer) =>
  switch (maybeBuffer) {
  | Some(buffer) => {
      ...view,
      viewLines: Buffer.getNumberOfLines(buffer),
      maxLineLength: _getMaxLineLength(buffer),
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
