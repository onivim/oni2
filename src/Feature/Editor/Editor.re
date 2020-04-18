open EditorCoreTypes;
open Oni_Core;

let lastId = ref(0);

[@deriving show]
type t = {
  bufferId: int,
  editorId: EditorId.t,
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
  font: [@opaque] Service_Font.font,
  pixelWidth: int,
  pixelHeight: int,
};

let create = (~font, ~bufferId=0, ()) => {
  let id = lastId^;
  incr(lastId);

  {
    editorId: id,
    bufferId,
    scrollX: 0.,
    scrollY: 0.,
    minimapMaxColumnWidth: Constants.minimapMaxColumn,
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
    font,
    pixelWidth: 1,
    pixelHeight: 1,
  };
};

type scrollbarMetrics = {
  visible: bool,
  thumbSize: int,
  thumbOffset: int,
};

let getVimCursors = model => model.cursors;

let mapCursor = (~position: Vim.Cursor.t, ~buffer) => {
  let byte = position.column |> Index.toZeroBased;
  let line = position.line |> Index.toZeroBased;

  let bufferLine = Buffer.getLine(line, buffer);

  let column = BufferLine.getIndex(~byte, bufferLine);

  Location.{line: Index.(zero + line), column: Index.(zero + column)};
};

let getCharacterUnderCursor = (~buffer, editor) => {
  None;
};

let getPrimaryCursor = (~buffer, editor) =>
  switch (editor.cursors) {
  | [cursor, ..._] => mapCursor(~position=cursor, ~buffer)
  | [] => Location.{line: Index.zero, column: Index.zero}
  };

let getId = model => model.editorId;

let getLineHeight = editor => editor.font.measuredHeight;
let getCharacterWidth = editor => editor.font.measuredWidth;

let pixelPositionToLineColumn = (view, pixelX, pixelY) => {
  let line = int_of_float((pixelY +. view.scrollY) /. getLineHeight(view));
  let column =
    int_of_float((pixelX +. view.scrollX) /. getCharacterWidth(view));

  (line, column);
};

let getVisibleView = editor => {
  let {pixelHeight, _} = editor;
  int_of_float(float_of_int(pixelHeight) /. getLineHeight(editor));
};

let getTotalSizeInPixels = editor =>
  int_of_float(float_of_int(editor.viewLines) *. getLineHeight(editor));

let getVerticalScrollbarMetrics = (view, scrollBarHeight) => {
  let {pixelHeight, _} = view;
  let totalViewSizeInPixels =
    float_of_int(getTotalSizeInPixels(view) + pixelHeight);
  let thumbPercentage = float_of_int(pixelHeight) /. totalViewSizeInPixels;
  let thumbSize =
    int_of_float(thumbPercentage *. float_of_int(scrollBarHeight));

  let topF = view.scrollY /. totalViewSizeInPixels;
  let thumbOffset = int_of_float(topF *. float_of_int(scrollBarHeight));

  {thumbSize, thumbOffset, visible: true};
};

let getHorizontalScrollbarMetrics = (view, availableWidth) => {
  let totalViewWidthInPixels =
    float_of_int(view.maxLineLength) *. getCharacterWidth(view);
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

let getLayout = view => {
  let {pixelWidth, pixelHeight, _} = view;
  let layout: EditorLayout.t =
    EditorLayout.getLayout(
      ~maxMinimapCharacters=view.minimapMaxColumnWidth,
      ~pixelWidth=float_of_int(pixelWidth),
      ~pixelHeight=float_of_int(pixelHeight),
      ~isMinimapShown=true,
      ~characterWidth=getCharacterWidth(view),
      ~characterHeight=getLineHeight(view),
      ~bufferLineCount=view.viewLines,
      (),
    );

  layout;
};

let getLeftVisibleColumn = view => {
  int_of_float(view.scrollX /. getCharacterWidth(view));
};

let getTopVisibleLine = view =>
  int_of_float(view.scrollY /. getLineHeight(view)) + 1;

let getBottomVisibleLine = view => {
  let absoluteBottomLine =
    int_of_float(
      (view.scrollY +. float_of_int(view.pixelHeight)) /. getLineHeight(view),
    );

  absoluteBottomLine > view.viewLines ? view.viewLines : absoluteBottomLine;
};

let setFont = (~font, editor) => {...editor, font};

let setSize = (~pixelWidth, ~pixelHeight, editor) => {
  ...editor,
  pixelWidth,
  pixelHeight,
};
