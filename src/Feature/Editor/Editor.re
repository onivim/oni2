open EditorCoreTypes;
open Oni_Core;

let lastId = ref(0);

type pixelPosition = {
  pixelX: float,
  pixelY: float,
};

type viewTokens = {
  tokens: list(BufferViewTokenizer.t),
  byteOffset: int,
  characterOffset: int,
};

[@deriving show]
// TODO: This type needs to be private, so we can maintain invariants with the `EditorBuffer.t` and computed properties
type t = {
  buffer: [@opaque] EditorBuffer.t,
  editorId: EditorId.t,
  scrollX: float,
  scrollY: float,
  minimapMaxColumnWidth: int,
  minimapScrollY: float,
  cursors: [@opaque] list(Vim.Cursor.t),
  selection: [@opaque] VisualRange.t,
  font: [@opaque] Service_Font.font,
  pixelWidth: int,
  pixelHeight: int,
  wrapping: [@opaque] Wrapping.t,
};

let totalViewLines = ({wrapping, _}) => wrapping |> Wrapping.numberOfLines;
let maxLineLength = ({wrapping, _}) => wrapping |> Wrapping.maxLineLength;
let selection = ({selection, _}) => selection;
let setSelection = (~selection, editor) => {...editor, selection};
let visiblePixelWidth = ({pixelWidth, _}) => pixelWidth;
let visiblePixelHeight = ({pixelHeight, _}) => pixelHeight;
let scrollY = ({scrollY, _}) => scrollY;
let scrollX = ({scrollX, _}) => scrollX;
let minimapScrollY = ({minimapScrollY, _}) => minimapScrollY;
let lineHeightInPixels = ({font, _}) => font.measuredHeight;
let characterWidthInPixels = ({font, _}) => font.measuredWidth;
let font = ({font, _}) => font;

let bufferLineByteToPixel =
    (~line, ~byteIndex, {scrollX, scrollY, buffer, font, wrapping, _}) => {
  let lineCount = EditorBuffer.numberOfLines(buffer);
  if (line < 0 || line >= lineCount) {
    ({pixelX: 0., pixelY: 0.}, 0.);
  } else {
    let bufferLine = buffer |> EditorBuffer.line(line);

    let viewLine = 
    Wrapping.bufferLineByteToViewLine(~line, ~byteIndex, wrapping);

    let {characterOffset, _}: Wrapping.bufferPosition =
    Wrapping.viewLineToBufferPosition(~line=viewLine, wrapping);

    let (startPosition, _) = BufferLine.getPositionAndWidth(
      ~index=characterOffset, bufferLine
    );

    let index = BufferLine.getIndex(~byte=byteIndex, bufferLine);
    let (characterOffset, width) =
      BufferLine.getPositionAndWidth(~index, bufferLine);

    let pixelX = font.measuredWidth *. float(characterOffset - startPosition) -. scrollX +. 0.5;

    let pixelY = font.measuredHeight *. float(viewLine) -. scrollY +. 0.5;

    ({pixelX, pixelY}, float(width) *. font.measuredWidth);
  };
};

let viewTokens = (~line, ~position, editor) => {
  ignore(line);
  ignore(position);
//  let bufferPosition = Wrapping.viewLineToBufferPosition(
//    ~line,
//    editor.wrapping,
//  );

  //let contents = editor.buffer |> EditorBuffer.line(bufferPosition.line);
  let tokens = [];

  {tokens, byteOffset: 0, characterOffset: 0};
};

let bufferLineCharacterToPixel =
    (~line, ~characterIndex, {scrollX, scrollY, buffer, font, _} as editor) => {
  let lineCount = EditorBuffer.numberOfLines(buffer);
  if (line < 0 || line >= lineCount) {
    ({pixelX: 0., pixelY: 0.}, 0.);
  } else {
    let byteIndex =
      buffer
      |> EditorBuffer.line(line)
      |> BufferLine.getByteFromIndex(~index=characterIndex);

      bufferLineByteToPixel(~line, ~byteIndex, editor);
  };
};

let create = (~wrap=WordWrap.none, ~font, ~buffer, ()) => {
  let id = lastId^;
  incr(lastId);

  {
    editorId: id,
    buffer,
    scrollX: 0.,
    scrollY: 0.,
    minimapMaxColumnWidth: Constants.minimapMaxColumn,
    minimapScrollY: 0.,
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
    wrapping: Wrapping.make(~wrap, ~buffer),
  };
};

let copy = editor => create(~font=editor.font, ~buffer=editor.buffer, ());

type scrollbarMetrics = {
  visible: bool,
  thumbSize: int,
  thumbOffset: int,
};

let getVimCursors = ({cursors, _}) => cursors;
let setVimCursors = (~cursors, editor) => {...editor, cursors};

let getNearestMatchingPair = (~location: Location.t, ~pairs, {buffer, _}) => {
  BracketMatch.findFirst(
    ~buffer,
    ~line=location.line |> Index.toZeroBased,
    ~index=location.column |> Index.toZeroBased,
    ~pairs,
  )
  |> Option.map(({start, stop}: BracketMatch.pair) =>
       (
         Location.{
           line: start.line |> Index.fromZeroBased,
           column: start.index |> Index.fromZeroBased,
         },
         Location.{
           line: stop.line |> Index.fromZeroBased,
           column: stop.index |> Index.fromZeroBased,
         },
       )
     );
};

let mapCursor = (~position: Vim.Cursor.t, editor) => {
  let byte = position.column |> Index.toZeroBased;
  let line = position.line |> Index.toZeroBased;

  let bufferLineCount = EditorBuffer.numberOfLines(editor.buffer);

  if (line < bufferLineCount) {
    let bufferLine = EditorBuffer.line(line, editor.buffer);

    let column = BufferLine.getIndex(~byte, bufferLine);

    Location.{line: Index.(zero + line), column: Index.(zero + column)};
  } else {
    Location.{line: Index.zero, column: Index.zero};
  };
};

let getCharacterBehindCursor = ({cursors, buffer, _}) => {
  switch (cursors) {
  | [] => None
  | [cursor, ..._] =>
    let byte = cursor.column |> Index.toZeroBased;
    let line = cursor.line |> Index.toZeroBased;

    let bufferLineCount = EditorBuffer.numberOfLines(buffer);

    if (line < bufferLineCount) {
      let bufferLine = EditorBuffer.line(line, buffer);
      let index = max(0, BufferLine.getIndex(~byte, bufferLine) - 1);
      try(Some(BufferLine.getUcharExn(~index, bufferLine))) {
      | _exn => None
      };
    } else {
      None;
    };
  };
};

let getCharacterUnderCursor = ({cursors, buffer, _}) => {
  switch (cursors) {
  | [] => None
  | [cursor, ..._] =>
    let byte = cursor.column |> Index.toZeroBased;
    let line = cursor.line |> Index.toZeroBased;

    let bufferLineCount = EditorBuffer.numberOfLines(buffer);

    if (line < bufferLineCount) {
      let bufferLine = EditorBuffer.line(line, buffer);
      let index = BufferLine.getIndex(~byte, bufferLine);
      try(Some(BufferLine.getUcharExn(~index, bufferLine))) {
      | _exn => None
      };
    } else {
      None;
    };
  };
};

let getPrimaryCursor = editor =>
  switch (editor.cursors) {
  | [cursor, ..._] => mapCursor(~position=cursor, editor)
  | [] => Location.{line: Index.zero, column: Index.zero}
  };

let selectionOrCursorRange = editor => {
  switch (editor.selection.mode) {
  | None =>
    let pos = getPrimaryCursor(editor);
    let range =
      Range.{
        start: Location.{line: pos.line, column: Index.zero},
        stop: Location.{line: Index.(pos.line + 1), column: Index.zero},
      };
    range;
  | Line
  | Block
  | Character => editor.selection.range
  };
};

let getId = model => model.editorId;

let getLineHeight = editor => editor.font.measuredHeight;
let getCharacterWidth = editor => editor.font.measuredWidth;

let getVisibleView = editor => {
  let {pixelHeight, _} = editor;
  int_of_float(float_of_int(pixelHeight) /. getLineHeight(editor));
};

let getTotalHeightInPixels = editor => {
  let viewLines = editor |> totalViewLines;
  int_of_float(float_of_int(viewLines) *. getLineHeight(editor));
  }

let getTotalWidthInPixels = editor => {
    let maxLineLength = editor |> maxLineLength;
  int_of_float(
    float_of_int(maxLineLength) *. getCharacterWidth(editor),
  );
};

let getVerticalScrollbarMetrics = (view, scrollBarHeight) => {
  let {pixelHeight, _} = view;
  let totalViewSizeInPixels =
    float_of_int(getTotalHeightInPixels(view) + pixelHeight);
  let thumbPercentage = float_of_int(pixelHeight) /. totalViewSizeInPixels;
  let thumbSize =
    int_of_float(thumbPercentage *. float_of_int(scrollBarHeight));

  let topF = view.scrollY /. totalViewSizeInPixels;
  let thumbOffset = int_of_float(topF *. float_of_int(scrollBarHeight));

  {thumbSize, thumbOffset, visible: true};
};

let getHorizontalScrollbarMetrics = (editor, availableWidth) => {
  
  let maxLineLength = editor |> maxLineLength;
  let availableWidthF = float_of_int(availableWidth);
  let totalViewWidthInPixels =
    float_of_int(maxLineLength + 1) *. getCharacterWidth(editor);
  //+. availableWidthF;

  totalViewWidthInPixels <= availableWidthF
    ? {visible: false, thumbSize: 0, thumbOffset: 0}
    : {
      let thumbPercentage = availableWidthF /. totalViewWidthInPixels;
      let thumbSize = int_of_float(thumbPercentage *. availableWidthF);

      let topF = editor.scrollX /. totalViewWidthInPixels;
      let thumbOffset = int_of_float(topF *. availableWidthF);

      {thumbSize, thumbOffset, visible: true};
    };
};

let getLayout =
    (~showLineNumbers, ~isMinimapShown, ~maxMinimapCharacters, editor) => {
  let viewLines = editor |> totalViewLines;
  let {pixelWidth, pixelHeight, _} = editor;
  let layout: EditorLayout.t =
    EditorLayout.getLayout(
      ~showLineNumbers,
      ~isMinimapShown,
      ~maxMinimapCharacters,
      ~pixelWidth=float_of_int(pixelWidth),
      ~pixelHeight=float_of_int(pixelHeight),
      ~characterWidth=getCharacterWidth(editor),
      ~characterHeight=getLineHeight(editor),
      ~bufferLineCount=viewLines,
      (),
    );

  layout;
};

let getLeftVisibleColumn = view => {
  int_of_float(view.scrollX /. getCharacterWidth(view));
};

let getTopVisibleLine = view =>
  int_of_float(view.scrollY /. getLineHeight(view)) + 1;

let getBottomVisibleLine = editor => {
  let absoluteBottomLine =
    int_of_float(
      (editor.scrollY +. float_of_int(editor.pixelHeight)) /. getLineHeight(editor),
    );

  let viewLines = editor |> totalViewLines;
  absoluteBottomLine > viewLines ? viewLines : absoluteBottomLine;
};

let setFont = (~font, editor) => {...editor, font};

let setSize = (~pixelWidth, ~pixelHeight, editor) => {
  ...editor,
  pixelWidth,
  pixelHeight,
};

let scrollToPixelY = (~pixelY as newScrollY, editor) => {
  let viewLines = editor |> totalViewLines;
  let {pixelHeight, _} = editor;
  let newScrollY = max(0., newScrollY);
  let availableScroll =
    max(float_of_int(viewLines - 1), 0.) *. getLineHeight(editor);
  let newScrollY = min(newScrollY, availableScroll);

  let scrollPercentage =
    newScrollY /. (availableScroll -. float_of_int(pixelHeight));
  let minimapLineSize =
    Constants.minimapCharacterWidth + Constants.minimapCharacterHeight;
  let linesInMinimap = pixelHeight / minimapLineSize;
  let availableMinimapScroll =
    max(viewLines - linesInMinimap, 0) * minimapLineSize;
  let newMinimapScroll =
    scrollPercentage *. float_of_int(availableMinimapScroll);

  {...editor, minimapScrollY: newMinimapScroll, scrollY: newScrollY};
};

let scrollToLine = (~line, view) => {
  let pixelY = float_of_int(line) *. getLineHeight(view);
  scrollToPixelY(~pixelY, view);
};

let scrollToPixelX = (~pixelX as newScrollX, editor) => {
  let maxLineLength = editor |> maxLineLength;
  let newScrollX = max(0., newScrollX);

  let availableScroll =
    max(0., float_of_int(maxLineLength) *. getCharacterWidth(editor));
  let scrollX = min(newScrollX, availableScroll);

  {...editor, scrollX};
};

let scrollDeltaPixelX = (~pixelX, editor) => {
  let pixelX = editor.scrollX +. pixelX;
  scrollToPixelX(~pixelX, editor);
};

let scrollToColumn = (~column, view) => {
  let pixelX = float_of_int(column) *. getCharacterWidth(view);
  scrollToPixelX(~pixelX, view);
};

let scrollDeltaPixelY = (~pixelY, view) => {
  let pixelY = view.scrollY +. pixelY;
  scrollToPixelY(~pixelY, view);
};

let scrollToPixelXY = (~pixelX as newScrollX, ~pixelY as newScrollY, view) => {
  let {scrollX, _} = scrollToPixelX(~pixelX=newScrollX, view);
  let {scrollY, minimapScrollY, _} =
    scrollToPixelY(~pixelY=newScrollY, view);

  {...view, scrollX, scrollY, minimapScrollY};
};

let scrollDeltaPixelXY = (~pixelX, ~pixelY, view) => {
  let {scrollX, _} = scrollDeltaPixelX(~pixelX, view);
  let {scrollY, minimapScrollY, _} = scrollDeltaPixelY(~pixelY, view);

  {...view, scrollX, scrollY, minimapScrollY};
};

// PROJECTION

let project = (~line, ~column: int, ~pixelWidth: int, ~pixelHeight, editor) => {
  // TODO: Horizontal scrolling
  ignore(column);
  ignore(pixelWidth);

  let editorPixelY = float_of_int(line) *. editor.font.measuredHeight;
  let totalEditorHeight = getTotalHeightInPixels(editor) |> float_of_int;
  let transformedPixelY =
    editorPixelY
    /. (totalEditorHeight +. float_of_int(editor.pixelHeight))
    *. float_of_int(pixelHeight);

  (0., transformedPixelY);
};

let projectLine = (~line, ~pixelHeight, editor) => {
  let (_x, y) =
    project(~line, ~column=0, ~pixelWidth=1, ~pixelHeight, editor);
  y;
};

let unprojectToPixel =
    (~pixelX: float, ~pixelY, ~pixelWidth: int, ~pixelHeight, editor) => {
  let totalWidth = getTotalWidthInPixels(editor) |> float_of_int;
  let x = totalWidth *. pixelX /. float_of_int(pixelWidth);

  let totalHeight = getTotalHeightInPixels(editor) |> float_of_int;
  let y = totalHeight *. pixelY /. float_of_int(pixelHeight);

  (x, y);
};

let getBufferId = ({buffer, _}) => EditorBuffer.id(buffer);

let updateBuffer = (~update, ~buffer, editor) => {
  {
    ...editor,
    buffer,
    wrapping: Wrapping.update(~update, ~newBuffer=buffer, editor.wrapping),
  };
};

module Slow = {
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
      let byte =
        BufferLine.Slow.getByteFromPosition(~position=rawColumn, bufferLine);
      (line, byte);
    } else {
      (
        // Empty buffer
        0,
        0,
      );
    };
  };
};
