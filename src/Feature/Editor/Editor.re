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

  let bufferLineCount = Buffer.getNumberOfLines(buffer);

  if (line < bufferLineCount) {
    let bufferLine = Buffer.getLine(line, buffer);

    let column = BufferLine.getIndex(~byte, bufferLine);

    Location.{line: Index.(zero + line), column: Index.(zero + column)};
  } else {
    Location.{line: Index.zero, column: Index.zero};
  };
};

let getCharacterUnderCursor = (~buffer, editor) => {
  switch (editor.cursors) {
  | [] => None
  | [cursor, ..._] =>
    let byte = cursor.column |> Index.toZeroBased;
    let line = cursor.line |> Index.toZeroBased;

    let bufferLineCount = Buffer.getNumberOfLines(buffer);

    if (line < bufferLineCount) {
      let bufferLine = Buffer.getLine(line, buffer);
      let index = BufferLine.getIndex(~byte, bufferLine);
      let character = BufferLine.getUcharExn(~index, bufferLine);
      Some(character);
    } else {
      None;
    };
  };
};

let getPrimaryCursor = (~buffer, editor) =>
  switch (editor.cursors) {
  | [cursor, ..._] => mapCursor(~position=cursor, ~buffer)
  | [] => Location.{line: Index.zero, column: Index.zero}
  };

let getId = model => model.editorId;

let getLineHeight = editor => editor.font.measuredHeight;
let getCharacterWidth = editor => editor.font.measuredWidth;

let pixelPositionToBufferLineByte =
    (~buffer, ~pixelX: float, ~pixelY: float, view) => {
  let rawLine =
    int_of_float((pixelY +. view.scrollY) /. getLineHeight(view));
  let rawColumn =
    int_of_float((pixelX +. view.scrollX) /. getCharacterWidth(view));

  let totalLinesInBuffer = Buffer.getNumberOfLines(buffer);

  let line =
    if (rawLine >= totalLinesInBuffer) {
      max(0, totalLinesInBuffer - 1);
    } else {
      rawLine;
    };

  if (line >= 0 && line < totalLinesInBuffer) {
    let bufferLine = Buffer.getLine(line, buffer);
    let byte = BufferLine.getByte(~index=rawColumn, bufferLine);
    (line, byte);
  } else {
    (
      // Empty buffer
      0,
      0,
    );
  };
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

let scrollToPixelY = (~pixelY as newScrollY, view) => {
  let {pixelHeight, _} = view;
  let newScrollY = max(0., newScrollY);
  let availableScroll =
    max(float_of_int(view.viewLines - 1), 0.) *. getLineHeight(view);
  let newScrollY = min(newScrollY, availableScroll);

  let scrollPercentage =
    newScrollY /. (availableScroll -. float_of_int(pixelHeight));
  let minimapLineSize =
    Constants.minimapCharacterWidth + Constants.minimapCharacterHeight;
  let linesInMinimap = pixelHeight / minimapLineSize;
  let availableMinimapScroll =
    max(view.viewLines - linesInMinimap, 0) * minimapLineSize;
  let newMinimapScroll =
    scrollPercentage *. float_of_int(availableMinimapScroll);

  {...view, minimapScrollY: newMinimapScroll, scrollY: newScrollY};
};

let scrollToLine = (~line, view) => {
  let pixelY = float_of_int(line) *. getLineHeight(view);
  scrollToPixelY(~pixelY, view);
};

let scrollToPixelX = (~pixelX as newScrollX, view) => {
  let newScrollX = max(0., newScrollX);

  let availableScroll =
    max(
      0.,
      float_of_int(view.maxLineLength)
      *. getCharacterWidth(view)
      -. float(view.pixelWidth),
    );
  let scrollX = min(newScrollX, availableScroll);

  {...view, scrollX};
};

let scrollToColumn = (~column, view) => {
  let pixelX = float_of_int(column) *. getCharacterWidth(view);
  scrollToPixelX(~pixelX, view);
};

let scrollDeltaPixelY = (~pixelY, view) => {
  let pixelY = view.scrollY +. pixelY;
  scrollToPixelY(~pixelY, view);
};
