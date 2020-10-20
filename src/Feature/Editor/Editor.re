open EditorCoreTypes;
open Oni_Core;

module GlobalState = {
  let lastId = ref(0);

  let generateId = () => {
    let id = lastId^;
    incr(lastId);
    id;
  };
};

module WrapMode = {
  [@deriving show]
  type t =
    | NoWrap
    | Viewport;
};

module WrapState = {
  [@deriving show]
  type t =
    | NoWrap({
        [@opaque]
        wrapping: Wrapping.t,
      })
    | Viewport({
        lastWrapPixels: float,
        [@opaque]
        wrapping: Wrapping.t,
      });

  let make = (~pixelWidth: float, ~wrapMode: WrapMode.t, ~buffer) => {
    switch (wrapMode) {
    | NoWrap =>
      NoWrap({wrapping: Wrapping.make(~wrap=WordWrap.none, ~buffer)})
    | Viewport =>
      Viewport({
        lastWrapPixels: pixelWidth,
        wrapping:
          Wrapping.make(~wrap=WordWrap.fixed(~pixels=pixelWidth), ~buffer),
      })
    };
  };

  let wrapping =
    fun
    | NoWrap({wrapping}) => wrapping
    | Viewport({wrapping, _}) => wrapping;

  let resize = (~pixelWidth: float, ~buffer, wrapState) => {
    switch (wrapState) {
    // All the cases where we don't need to update wrapping...
    | NoWrap(_) as nowrap => nowrap
    | Viewport({lastWrapPixels, _}) when lastWrapPixels != pixelWidth =>
      let wrapping =
        Wrapping.make(~wrap=WordWrap.fixed(~pixels=pixelWidth), ~buffer);
      Viewport({lastWrapPixels: pixelWidth, wrapping});
    | Viewport(_) as viewport => viewport
    };
  };

  let map = f =>
    fun
    | NoWrap({wrapping}) => NoWrap({wrapping: f(wrapping)})
    | Viewport({wrapping, lastWrapPixels}) =>
      Viewport({wrapping: f(wrapping), lastWrapPixels});

  let update = (~update, ~buffer, wrapState) => {
    wrapState |> map(Wrapping.update(~update, ~newBuffer=buffer));
  };
};

[@deriving show]
type yankHighlight = {
  key: [@opaque] Brisk_reconciler.Key.t,
  pixelRanges: list(PixelRange.t),
};

[@deriving show]
type t = {
  key: [@opaque] Brisk_reconciler.Key.t,
  buffer: [@opaque] EditorBuffer.t,
  editorId: EditorId.t,
  lineNumbers: [ | `Off | `On | `Relative | `RelativeOnly],
  lineHeight: LineHeight.t,
  scrollX: float,
  scrollY: float,
  isScrollAnimated: bool,
  isMinimapEnabled: bool,
  minimapMaxColumnWidth: int,
  minimapScrollY: float,
  mode: [@opaque] Vim.Mode.t,
  pixelWidth: int,
  pixelHeight: int,
  yankHighlight: option(yankHighlight),
  wrapMode: WrapMode.t,
  wrapState: WrapState.t,
  wrapPadding: option(float),
  // Number of lines to preserve before or after the cursor, when scrolling.
  // Like the `scrolloff` vim setting or the `editor.cursorSurroundingLines` VSCode setting.
  verticalScrollMargin: int,
};

let key = ({key, _}) => key;
let selection = ({mode, _}) =>
  switch (mode) {
  | Visual(range) => Some(Oni_Core.VisualRange.ofVim(range))
  | Select(range) => Some(Oni_Core.VisualRange.ofVim(range))
  | _ => None
  };
let visiblePixelWidth = ({pixelWidth, _}) => pixelWidth;
let visiblePixelHeight = ({pixelHeight, _}) => pixelHeight;
let scrollY = ({scrollY, _}) => scrollY;
let scrollX = ({scrollX, _}) => scrollX;
let minimapScrollY = ({minimapScrollY, _}) => minimapScrollY;
let lineHeightInPixels = ({buffer, lineHeight, _}) =>
  lineHeight
  |> LineHeight.calculate(
       ~measuredFontHeight=EditorBuffer.font(buffer).measuredHeight,
     );

let linePaddingInPixels = ({buffer, _} as editor) =>
  (lineHeightInPixels(editor) -. EditorBuffer.font(buffer).measuredHeight)
  /. 2.;
let characterWidthInPixels = ({buffer, _}) =>
  EditorBuffer.font(buffer).spaceWidth;
let font = ({buffer, _}) => EditorBuffer.font(buffer);

let setMinimap = (~enabled, ~maxColumn, editor) => {
  ...editor,
  isMinimapEnabled: enabled,
  minimapMaxColumnWidth: maxColumn,
};

let getBufferLineCount = ({buffer, _}) =>
  EditorBuffer.numberOfLines(buffer);

let isMinimapEnabled = ({isMinimapEnabled, _}) => isMinimapEnabled;
let isScrollAnimated = ({isScrollAnimated, _}) => isScrollAnimated;

let bufferBytePositionToPixel =
    (
      ~position: BytePosition.t,
      {scrollX, scrollY, buffer, wrapState, _} as editor,
    ) => {
  let lineCount = EditorBuffer.numberOfLines(buffer);
  let line = position.line |> EditorCoreTypes.LineNumber.toZeroBased;
  if (line < 0 || line >= lineCount) {
    ({x: 0., y: 0.}: PixelPosition.t, 0.);
  } else {
    let wrapping = wrapState |> WrapState.wrapping;
    let bufferLine = buffer |> EditorBuffer.line(line);

    let viewLine =
      Wrapping.bufferBytePositionToViewLine(~bytePosition=position, wrapping);

    let {characterOffset: viewStartIndex, _}: Wrapping.bufferPosition =
      Wrapping.viewLineToBufferPosition(~line=viewLine, wrapping);

    let (startPixel, _width) =
      BufferLine.getPixelPositionAndWidth(~index=viewStartIndex, bufferLine);

    let index = BufferLine.getIndex(~byte=position.byte, bufferLine);
    let (actualPixel, width) =
      BufferLine.getPixelPositionAndWidth(~index, bufferLine);

    let pixelX = actualPixel -. startPixel -. scrollX +. 0.5;
    let pixelY =
      lineHeightInPixels(editor) *. float(viewLine) -. scrollY +. 0.5;

    ({x: pixelX, y: pixelY}: PixelPosition.t, width);
  };
};

let yankHighlight = ({yankHighlight, _}) => yankHighlight;
let setYankHighlight = (~yankHighlight, editor) => {
  ...editor,
  yankHighlight: Some(yankHighlight),
};

let setWrapPadding = (~padding, editor) => {
  ...editor,
  wrapPadding: Some(padding),
};

let viewLineToBufferLine = (viewLine, editor) => {
  let wrapping = editor.wrapState |> WrapState.wrapping;
  let bufferPosition: Wrapping.bufferPosition =
    Wrapping.viewLineToBufferPosition(~line=viewLine, wrapping);

  bufferPosition.line;
};

let bufferBytePositionToViewLine = (bytePosition, editor) => {
  let wrapping = editor.wrapState |> WrapState.wrapping;
  Wrapping.bufferBytePositionToViewLine(~bytePosition, wrapping);
};

let viewLineIsPrimary = (viewLine, editor) => {
  let wrapping = editor.wrapState |> WrapState.wrapping;
  let bufferPosition: Wrapping.bufferPosition =
    Wrapping.viewLineToBufferPosition(~line=viewLine, wrapping);

  bufferPosition.byteOffset == ByteIndex.zero;
};

let viewTokens = (~line, ~scrollX, ~colorizer, editor) => {
  let wrapping = editor.wrapState |> WrapState.wrapping;
  let bufferPosition: Wrapping.bufferPosition =
    Wrapping.viewLineToBufferPosition(~line, wrapping);

  let startByte = bufferPosition.byteOffset;

  let bufferLine =
    EditorBuffer.line(
      bufferPosition.line |> EditorCoreTypes.LineNumber.toZeroBased,
      editor.buffer,
    );

  let viewStartByte =
    BufferLine.Slow.getByteFromPixel(
      ~relativeToByte=startByte,
      ~pixelX=scrollX,
      bufferLine,
    );

  let maxEndByte =
    BufferLine.Slow.getByteFromPixel(
      ~relativeToByte=viewStartByte,
      ~pixelX=float(editor.pixelWidth),
      bufferLine,
    );

  let totalViewLines = Wrapping.numberOfLines(wrapping);

  let viewEndByte =
    if (line < totalViewLines - 1) {
      let nextLineBufferPosition =
        Wrapping.viewLineToBufferPosition(~line=line + 1, wrapping);

      if (nextLineBufferPosition.line == bufferPosition.line
          && nextLineBufferPosition.byteOffset > bufferPosition.byteOffset) {
        nextLineBufferPosition.byteOffset;
      } else {
        maxEndByte;
      };
    } else {
      maxEndByte;
    };

  let viewStartIndex = BufferLine.getIndex(~byte=viewStartByte, bufferLine);
  let viewEndIndex = BufferLine.getIndex(~byte=viewEndByte, bufferLine);

  BufferViewTokenizer.tokenize(
    ~start=viewStartIndex,
    ~stop=viewEndIndex,
    bufferLine,
    colorizer(~startByte=viewStartByte),
  );
};

let bufferCharacterPositionToPixel =
    (~position: CharacterPosition.t, {buffer, _} as editor) => {
  let lineCount = EditorBuffer.numberOfLines(buffer);
  let line = position.line |> EditorCoreTypes.LineNumber.toZeroBased;
  if (line < 0 || line >= lineCount) {
    ({x: 0., y: 0.}: PixelPosition.t, 0.);
  } else {
    let byteIndex =
      buffer
      |> EditorBuffer.line(line)
      |> BufferLine.getByteFromIndex(~index=position.character);

    bufferBytePositionToPixel(
      ~position=BytePosition.{line: position.line, byte: byteIndex},
      editor,
    );
  };
};

let create = (~config, ~buffer, ()) => {
  let id = GlobalState.generateId();
  let key = Brisk_reconciler.Key.create();

  let isMinimapEnabled = EditorConfiguration.Minimap.enabled.get(config);
  let minimapMaxColumnWidth =
    EditorConfiguration.Minimap.maxColumn.get(config);
  let lineNumbers = EditorConfiguration.lineNumbers.get(config);
  let lineHeight = EditorConfiguration.lineHeight.get(config);
  let wrapMode =
    EditorConfiguration.Experimental.wordWrap.get(config) == `On
      ? WrapMode.Viewport : WrapMode.NoWrap;

  let wrapState = WrapState.make(~pixelWidth=1000., ~wrapMode, ~buffer);

  {
    editorId: id,
    key,
    lineHeight,
    lineNumbers,
    isMinimapEnabled,
    isScrollAnimated: false,
    buffer,
    scrollX: 0.,
    scrollY: 0.,
    minimapMaxColumnWidth,
    minimapScrollY: 0.,
    /*
     * We need an initial editor size, otherwise we'll immediately scroll the view
     * if a buffer loads prior to our first render.
     */
    mode:
      Vim.Mode.Normal({
        cursor:
          BytePosition.{
            line: EditorCoreTypes.LineNumber.zero,
            byte: ByteIndex.zero,
          },
      }),
    pixelWidth: 1,
    pixelHeight: 1,
    yankHighlight: None,
    wrapState,
    wrapMode,
    wrapPadding: None,
    verticalScrollMargin: 1,
  };
};

let cursors = ({mode, _}) => Vim.Mode.cursors(mode);

let totalViewLines = ({wrapState, _}) =>
  wrapState |> WrapState.wrapping |> Wrapping.numberOfLines;
let maxLineLength = ({wrapState, _}) =>
  wrapState |> WrapState.wrapping |> Wrapping.maxLineLength;

let getTopVisibleBufferLine = editor => {
  let topViewLine =
    int_of_float(editor.scrollY /. lineHeightInPixels(editor));
  viewLineToBufferLine(topViewLine, editor);
};

let getBottomVisibleBufferLine = editor => {
  let absoluteBottomLine =
    int_of_float(
      (editor.scrollY +. float_of_int(editor.pixelHeight))
      /. lineHeightInPixels(editor),
    );

  let viewLines = editor |> totalViewLines;

  let viewBottomLine =
    absoluteBottomLine >= viewLines ? viewLines - 1 : absoluteBottomLine;
  viewLineToBufferLine(viewBottomLine, editor);
};

let copy = editor => {
  let id = GlobalState.generateId();
  let key = Brisk_reconciler.Key.create();

  {...editor, key, editorId: id};
};

type scrollbarMetrics = {
  visible: bool,
  thumbSize: int,
  thumbOffset: int,
};

let getCursors = ({mode, _}) => Vim.Mode.cursors(mode);

let getNearestMatchingPair =
    (~characterPosition: CharacterPosition.t, ~pairs, {buffer, _}) => {
  BracketMatch.findFirst(~buffer, ~characterPosition, ~pairs)
  |> Option.map(({start, stop}: BracketMatch.pair) => (start, stop));
};

let byteToCharacter = (position: BytePosition.t, editor) => {
  let line = position.line |> EditorCoreTypes.LineNumber.toZeroBased;

  let bufferLineCount = EditorBuffer.numberOfLines(editor.buffer);

  if (line < bufferLineCount) {
    let bufferLine = EditorBuffer.line(line, editor.buffer);
    let character = BufferLine.getIndex(~byte=position.byte, bufferLine);

    Some(
      EditorCoreTypes.(CharacterPosition.{line: position.line, character}),
    );
  } else {
    None;
  };
};

let byteRangeToCharacterRange = ({start, stop}: ByteRange.t, editor) => {
  let maybeCharacterStart = byteToCharacter(start, editor);
  let maybeCharacterStop = byteToCharacter(stop, editor);

  Oni_Core.Utility.OptionEx.map2(
    (start, stop) => {CharacterRange.{start, stop}},
    maybeCharacterStart,
    maybeCharacterStop,
  );
};

let characterToByte = (position: CharacterPosition.t, editor) => {
  let line = position.line |> EditorCoreTypes.LineNumber.toZeroBased;

  let bufferLineCount = EditorBuffer.numberOfLines(editor.buffer);

  if (line < bufferLineCount) {
    let bufferLine = EditorBuffer.line(line, editor.buffer);
    let byteIndex =
      BufferLine.getByteFromIndex(~index=position.character, bufferLine);

    Some(
      EditorCoreTypes.(BytePosition.{line: position.line, byte: byteIndex}),
    );
  } else {
    None;
  };
};
let getCharacterAtPosition = (~position: CharacterPosition.t, {buffer, _}) => {
  let line = EditorCoreTypes.LineNumber.toZeroBased(position.line);
  let bufferLineCount = EditorBuffer.numberOfLines(buffer);

  if (line < bufferLineCount) {
    let bufferLine = EditorBuffer.line(line, buffer);
    try(Some(BufferLine.getUcharExn(~index=position.character, bufferLine))) {
    | _exn => None
    };
  } else {
    None;
  };
};

let getCharacterBehindCursor = ({buffer, _} as editor) => {
  switch (cursors(editor)) {
  | [] => None
  | [cursor, ..._] =>
    let line = cursor.line |> EditorCoreTypes.LineNumber.toZeroBased;

    let bufferLineCount = EditorBuffer.numberOfLines(buffer);

    if (line < bufferLineCount) {
      let bufferLine = EditorBuffer.line(line, buffer);
      let index =
        max(
          0,
          CharacterIndex.toInt(
            BufferLine.getIndex(~byte=cursor.byte, bufferLine),
          )
          - 1,
        );
      try(
        Some(
          BufferLine.getUcharExn(
            ~index=CharacterIndex.ofInt(index),
            bufferLine,
          ),
        )
      ) {
      | _exn => None
      };
    } else {
      None;
    };
  };
};

//let byteToCharacter = (cursor: BytePosition.t, {buffer, _}) => {
//    let line = cursor.line |> EditorCoreTypes.LineNumber.toZeroBased;
//    //let line = cursor.line |> EditorCoreTypes.LineNumber.toZeroBased;
//    let bufferLineCount = EditorBuffer.numberOfLines(buffer);
//    if (line < bufferLineCount) {
//      let bufferLine = EditorBuffer.line(line, buffer);
//      let index = BufferLine.getIndex(~byte=cursor.byte, bufferLine);
//      Some(CharacterPosition.{
//        line: cursor.line,
//        character: index,
//      });
//      try(Some(BufferLine.getUcharExn(~index, bufferLine))) {
//      | _exn => None
//      };
//    } else {
//      None;
//    };
//}

let getCharacterUnderCursor = ({buffer, _} as editor) => {
  switch (cursors(editor)) {
  | [] => None
  | [cursor, ..._] =>
    let line = cursor.line |> EditorCoreTypes.LineNumber.toZeroBased;

    let bufferLineCount = EditorBuffer.numberOfLines(buffer);

    if (line < bufferLineCount) {
      let bufferLine = EditorBuffer.line(line, buffer);
      let index = BufferLine.getIndex(~byte=cursor.byte, bufferLine);
      try(Some(BufferLine.getUcharExn(~index, bufferLine))) {
      | _exn => None
      };
    } else {
      None;
    };
  };
};

let getPrimaryCursor = editor => {
  let maybeCharacterCursor =
    switch (cursors(editor)) {
    | [cursor, ..._] => byteToCharacter(cursor, editor)
    | [] => None
    };

  maybeCharacterCursor
  |> Option.value(
       ~default=
         CharacterPosition.{
           line: EditorCoreTypes.LineNumber.zero,
           character: CharacterIndex.zero,
         },
     );
};

let getPrimaryCursorByte = editor =>
  switch (cursors(editor)) {
  | [cursor, ..._] => cursor
  | [] =>
    BytePosition.{line: EditorCoreTypes.LineNumber.zero, byte: ByteIndex.zero}
  };
let selectionOrCursorRange = editor => {
  switch (selection(editor)) {
  | None =>
    let pos = getPrimaryCursorByte(editor);
    ByteRange.{
      start: BytePosition.{line: pos.line, byte: ByteIndex.zero},
      stop:
        BytePosition.{
          line: EditorCoreTypes.LineNumber.(pos.line + 1),
          byte: ByteIndex.zero,
        },
    };
  | Some(selection) => selection.range
  };
};

let setLineHeight = (~lineHeight, editor) => {...editor, lineHeight};

let setLineNumbers = (~lineNumbers, editor) => {...editor, lineNumbers};
let lineNumbers = ({lineNumbers, _}) => lineNumbers;

let getId = model => model.editorId;

let getCharacterWidth = ({buffer, _}) =>
  EditorBuffer.font(buffer).spaceWidth;

let getVisibleView = editor => {
  let {pixelHeight, _} = editor;
  int_of_float(float_of_int(pixelHeight) /. lineHeightInPixels(editor));
};

let getTotalHeightInPixels = editor => {
  let totalViewLines = editor |> totalViewLines;
  int_of_float(float_of_int(totalViewLines) *. lineHeightInPixels(editor));
};

let getTotalWidthInPixels = editor => {
  let maxLineLength = editor |> maxLineLength;
  int_of_float(float_of_int(maxLineLength) *. getCharacterWidth(editor));
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

let getLayout = editor => {
  let {
    pixelWidth,
    pixelHeight,
    isMinimapEnabled,
    lineNumbers,
    minimapMaxColumnWidth,
    _,
  } = editor;
  let layout: EditorLayout.t =
    EditorLayout.getLayout(
      ~showLineNumbers=lineNumbers != `Off,
      ~isMinimapShown=isMinimapEnabled,
      ~maxMinimapCharacters=minimapMaxColumnWidth,
      ~pixelWidth=float_of_int(pixelWidth),
      ~pixelHeight=float_of_int(pixelHeight),
      ~characterWidth=getCharacterWidth(editor),
      ~characterHeight=lineHeightInPixels(editor),
      ~bufferLineCount=editor |> totalViewLines,
      (),
    );

  layout;
};

let getMinimapWidthScaleFactor = editor => {
  let {bufferWidthInPixels, minimapWidthInPixels, _}: EditorLayout.t =
    getLayout(editor);

  if (bufferWidthInPixels != 0.) {
    float(minimapWidthInPixels) /. bufferWidthInPixels;
  } else {
    1.0;
  };
};

let exposePrimaryCursor = editor => {
  switch (cursors(editor)) {
  | [primaryCursor, ..._tail] =>
    let {bufferWidthInPixels, _}: EditorLayout.t = getLayout(editor);

    let pixelWidth = bufferWidthInPixels;

    let {pixelHeight, scrollX, scrollY, _} = editor;
    let pixelHeight = float(pixelHeight);

    let ({x: pixelX, y: pixelY}: PixelPosition.t, _width) =
      bufferBytePositionToPixel(~position=primaryCursor, editor);

    let scrollOffX = getCharacterWidth(editor) *. 2.;
    let scrollOffY =
      lineHeightInPixels(editor) *. float(editor.verticalScrollMargin);

    let availableX = pixelWidth -. scrollOffX;
    let availableY = pixelHeight -. scrollOffY;

    let adjustedScrollX =
      if (pixelX < 0.) {
        scrollX +. pixelX;
      } else if (pixelX >= availableX) {
        scrollX +. (pixelX -. availableX);
      } else {
        scrollX;
      };

    let adjustedScrollY =
      if (pixelY < scrollOffY) {
        scrollY -. scrollOffY +. pixelY;
      } else if (pixelY >= availableY) {
        scrollY +. (pixelY -. availableY);
      } else {
        scrollY;
      };

    {
      ...editor,
      scrollX: adjustedScrollX,
      scrollY: adjustedScrollY,
      isScrollAnimated: true,
    };

  | _ => editor
  };
};

let mode = ({mode, _}) => mode;

let setMode = (mode, editor) => {
  {...editor, mode} |> exposePrimaryCursor;
};

let getLeftVisibleColumn = view => {
  int_of_float(view.scrollX /. getCharacterWidth(view));
};

let getTokenAt =
    (~languageConfiguration, {line, character}: CharacterPosition.t, editor) => {
  let lineNumber = line |> EditorCoreTypes.LineNumber.toZeroBased;

  if (lineNumber < 0
      || lineNumber >= EditorBuffer.numberOfLines(editor.buffer)) {
    None;
  } else {
    let bufferLine = EditorBuffer.line(lineNumber, editor.buffer);
    let f = uchar =>
      LanguageConfiguration.isWordCharacter(uchar, languageConfiguration);
    let startIndex =
      BufferLine.traverse(
        ~f,
        ~direction=`Backwards,
        ~index=character,
        bufferLine,
      );
    let stopIndex =
      BufferLine.traverse(
        ~f,
        ~direction=`Forwards,
        ~index=character,
        bufferLine,
      );
    Some(
      CharacterRange.{
        start: CharacterPosition.{line, character: startIndex},
        stop: CharacterPosition.{line, character: stopIndex},
      },
    );
  };
};
let getContentPixelWidth = editor => {
  let layout: EditorLayout.t = getLayout(editor);
  layout.bufferWidthInPixels;
};

let setSize = (~pixelWidth, ~pixelHeight, editor) => {
  let editor' = {...editor, pixelWidth, pixelHeight};

  let contentPixelWidth = getContentPixelWidth(editor');

  let wrapPadding =
    switch (editor.wrapPadding) {
    | None => EditorBuffer.measure(Uchar.of_char('W'), editor.buffer)
    | Some(padding) => padding
    };

  let wrapWidth = contentPixelWidth -. wrapPadding;
  let wrapState =
    WrapState.resize(
      ~pixelWidth=wrapWidth,
      ~buffer=editor'.buffer,
      editor'.wrapState,
    );

  {...editor', wrapState};
};

let scrollToPixelY = (~pixelY as newScrollY, editor) => {
  let {pixelHeight, _} = editor;
  let viewLines = editor |> totalViewLines;
  let newScrollY = max(0., newScrollY);
  let availableScroll =
    max(float_of_int(viewLines - 1), 0.) *. lineHeightInPixels(editor);
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

  {
    ...editor,
    isScrollAnimated: false,
    minimapScrollY: newMinimapScroll,
    scrollY: newScrollY,
  };
};

let animateScroll = editor => {...editor, isScrollAnimated: true};

let scrollToLine = (~line, view) => {
  let pixelY = float_of_int(line) *. lineHeightInPixels(view);
  {...scrollToPixelY(~pixelY, view), isScrollAnimated: true};
};

let scrollToPixelX = (~pixelX as newScrollX, editor) => {
  let maxLineLength = editor |> maxLineLength;
  let newScrollX = max(0., newScrollX);

  let availableScroll =
    max(0., float_of_int(maxLineLength) *. getCharacterWidth(editor));
  let scrollX = min(newScrollX, availableScroll);

  {...editor, isScrollAnimated: false, scrollX};
};

let scrollDeltaPixelX = (~pixelX, editor) => {
  let pixelX = editor.scrollX +. pixelX;
  scrollToPixelX(~pixelX, editor);
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

let scrollAndMoveCursor = (~deltaViewLines, editor) => {
  let wrapping = editor.wrapState |> WrapState.wrapping;
  let totalViewLines = Wrapping.numberOfLines(wrapping);

  let oldCursor = getPrimaryCursorByte(editor);

  let adjustCursor = (cursor: BytePosition.t) => {
    let currentViewLine =
      Wrapping.bufferBytePositionToViewLine(~bytePosition=cursor, wrapping);
    let newViewLine =
      Utility.IntEx.clamp(
        ~lo=0,
        ~hi=totalViewLines - 1,
        currentViewLine + deltaViewLines,
      );
    let {line: outLine, byteOffset, _}: Wrapping.bufferPosition =
      Wrapping.viewLineToBufferPosition(~line=newViewLine, wrapping);
    BytePosition.{line: outLine, byte: byteOffset};
  };

  let mode =
    switch (editor.mode) {
    // When scrolling in operator pending, cancel the pending operator
    | Operator({cursor, _}) => Vim.Mode.Normal({cursor: cursor})
    // Don't do anything for command line mode
    | CommandLine => CommandLine
    | Normal({cursor}) => Normal({cursor: adjustCursor(cursor)})
    | Visual(curr) =>
      Visual(Vim.VisualRange.{...curr, cursor: adjustCursor(curr.cursor)})
    | Select(curr) =>
      Select(Vim.VisualRange.{...curr, cursor: adjustCursor(curr.cursor)})
    | Replace({cursor}) => Replace({cursor: adjustCursor(cursor)})
    | Insert({cursors}) =>
      Insert({cursors: List.map(adjustCursor, cursors)})
    };

  // Move the cursor...
  let editor' = {...editor, mode};

  let newCursor = getPrimaryCursorByte(editor');
  // Figure out the delta in scrollY
  let ({y: oldY, _}: PixelPosition.t, _) =
    bufferBytePositionToPixel(~position=oldCursor, editor);

  let ({y: newY, _}: PixelPosition.t, _) =
    bufferBytePositionToPixel(~position=newCursor, editor');

  // Adjust scroll to compensate cursor position - keeping the cursor in the same
  // relative spot to scroll, after moving it.
  editor' |> scrollDeltaPixelY(~pixelY=newY -. oldY) |> animateScroll;
};

let scrollCenterCursorVertically = editor => {
  let cursor = getPrimaryCursorByte(editor);
  let (pixelPosition: PixelPosition.t, _) =
    bufferBytePositionToPixel(~position=cursor, editor);

  let heightInPixels = float(editor.pixelHeight);
  let pixelY = pixelPosition.y +. editor.scrollY -. heightInPixels /. 2.;
  scrollToPixelY(~pixelY, editor) |> animateScroll;
};

let scrollCursorTop = editor => {
  let cursor = getPrimaryCursorByte(editor);
  let (pixelPosition: PixelPosition.t, _) =
    bufferBytePositionToPixel(~position=cursor, editor);

  let pixelY = pixelPosition.y +. editor.scrollY;
  scrollToPixelY(~pixelY, editor) |> animateScroll;
};

let scrollCursorBottom = editor => {
  let cursor = getPrimaryCursorByte(editor);
  let (pixelPosition: PixelPosition.t, _) =
    bufferBytePositionToPixel(~position=cursor, editor);

  let heightInPixels = float(editor.pixelHeight);
  let pixelY =
    pixelPosition.y
    +. editor.scrollY
    -. (heightInPixels -. lineHeightInPixels(editor));
  scrollToPixelY(~pixelY, editor) |> animateScroll;
};

let scrollLines = (~count, editor) => {
  scrollAndMoveCursor(~deltaViewLines=count, editor);
};

let scrollHalfPage = (~count, editor) => {
  let lineDelta =
    count
    * int_of_float(
        float(editor.pixelHeight) /. lineHeightInPixels(editor) /. 2.,
      );
  scrollAndMoveCursor(~deltaViewLines=lineDelta, editor);
};

let scrollPage = (~count, editor) => {
  let lineDelta =
    count
    * int_of_float(float(editor.pixelHeight) /. lineHeightInPixels(editor));
  scrollAndMoveCursor(~deltaViewLines=lineDelta, editor);
};

// PROJECTION

let project = (~line, ~column: int, ~pixelWidth: int, ~pixelHeight, editor) => {
  // TODO: Horizontal scrolling
  ignore(column);
  ignore(pixelWidth);

  let lineIdx = EditorCoreTypes.LineNumber.toZeroBased(line);
  let editorPixelY = float_of_int(lineIdx) *. lineHeightInPixels(editor);
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
    wrapState: WrapState.update(~update, ~buffer, editor.wrapState),
  };
};

let setBuffer = (~buffer, editor) => {
  {
    ...editor,
    buffer,
    wrapState:
      WrapState.make(
        ~pixelWidth=getContentPixelWidth(editor),
        ~wrapMode=editor.wrapMode,
        ~buffer,
      ),
  };
};

let setWrapMode = (~wrapMode, editor) => {
  let pixelWidth = getContentPixelWidth(editor);
  {
    ...editor,
    wrapMode,
    wrapState: WrapState.make(~pixelWidth, ~wrapMode, ~buffer=editor.buffer),
  };
};

let configurationChanged = (~perFileTypeConfig, editor) => {
  let fileType =
    editor.buffer |> EditorBuffer.fileType |> Oni_Core.Buffer.FileType.toString;

  let config = perFileTypeConfig(~fileType);

  let wrapMode =
    EditorConfiguration.Experimental.wordWrap.get(config) == `On
      ? WrapMode.Viewport : WrapMode.NoWrap;

  editor
  |> setMinimap(
       ~enabled=EditorConfiguration.Minimap.enabled.get(config),
       ~maxColumn=EditorConfiguration.Minimap.maxColumn.get(config),
     )
  |> setLineHeight(~lineHeight=EditorConfiguration.lineHeight.get(config))
  |> setLineNumbers(
       ~lineNumbers=EditorConfiguration.lineNumbers.get(config),
     )
  |> setWrapMode(~wrapMode);
};

module Slow = {
  let pixelPositionToBytePosition =
      (~allowPast=false, ~pixelX: float, ~pixelY: float, view) => {
    let rawLine =
      int_of_float((pixelY +. view.scrollY) /. lineHeightInPixels(view));

    let wrapping = view.wrapState |> WrapState.wrapping;
    let rawLine =
      Utility.IntEx.clamp(
        ~lo=0,
        ~hi=Wrapping.numberOfLines(wrapping) - 1,
        rawLine,
      );
    let {line, byteOffset, _}: Wrapping.bufferPosition =
      Wrapping.viewLineToBufferPosition(~line=rawLine, wrapping);
    let totalLinesInBuffer = EditorBuffer.numberOfLines(view.buffer);

    let lineIdx = EditorCoreTypes.LineNumber.toZeroBased(line);
    //      if (rawLine >= totalLinesInBuffer) {
    //        max(0, totalLinesInBuffer - 1);
    //      } else {
    //        rawLine;
    //      };

    if (lineIdx >= 0 && lineIdx < totalLinesInBuffer) {
      let bufferLine = EditorBuffer.line(lineIdx, view.buffer);
      let byteIndex =
        BufferLine.Slow.getByteFromPixel(
          ~relativeToByte=byteOffset,
          ~pixelX=pixelX +. view.scrollX,
          bufferLine,
        );

      let bytePositionInBounds =
        BytePosition.{
          line: EditorCoreTypes.LineNumber.ofZeroBased(lineIdx),
          byte: byteIndex,
        };

      if (!allowPast
          && ByteIndex.toInt(byteIndex)
          > BufferLine.lengthInBytes(bufferLine)
          - 1) {
        BytePosition.{
          line: bytePositionInBounds.line,
          byte: ByteIndex.ofInt(BufferLine.lengthInBytes(bufferLine) - 1),
        };
      } else {
        bytePositionInBounds;
      };
    } else {
      BytePosition.{
        line: EditorCoreTypes.LineNumber.zero,
        byte: ByteIndex.zero,
      };
    };
  };
};

let moveScreenLines = (~position, ~count, editor) => {
  let ({x, y}: PixelPosition.t, _: float) =
    bufferBytePositionToPixel(~position, editor);

  let deltaY = float(count) *. lineHeightInPixels(editor);
  Slow.pixelPositionToBytePosition(
    ~allowPast=true,
    ~pixelX=x,
    ~pixelY=y +. deltaY,
    editor,
  );
};
