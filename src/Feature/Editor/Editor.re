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
    | Viewport
    | WrapColumn(int)
    | Bounded(int);
};

module WrapState = {
  [@deriving show]
  type t =
    | NoWrap({
        [@opaque]
        wrapping: Wrapping.t,
      })
    | Viewport({
        lastWrapColumn: int,
        [@opaque]
        wrapping: Wrapping.t,
      })
    | WrapColumn({
        wrapColumn: int,
        [@opaque]
        wrapping: Wrapping.t,
      })
    | Bounded({
        wrapColumn: int,
        lastWrapColumn: int,
        [@opaque]
        wrapping: Wrapping.t,
      });

  let make = (~wrapMode: WrapMode.t, ~buffer) => {
    let initialWrapping = Wrapping.make(~wrap=WordWrap.none, ~buffer);
    switch (wrapMode) {
    | NoWrap => NoWrap({wrapping: initialWrapping})
    | Viewport => Viewport({lastWrapColumn: 999, wrapping: initialWrapping})
    | WrapColumn(wrapColumn) =>
      WrapColumn({
        wrapColumn,
        wrapping: initialWrapping,
        // TODO:
        //Wrapping.make(~wrap=WordWrap.fixed(~columns=wrapColumn), ~buffer),
      })
    | Bounded(wrapColumn) =>
      Bounded({
        wrapColumn,
        lastWrapColumn: wrapColumn,
        wrapping: initialWrapping,
        // TODO:
        //Wrapping.make(~wrap=WordWrap.fixed(~columns=wrapColumn), ~buffer),
      })
    };
  };

  let wrapping =
    fun
    | NoWrap({wrapping}) => wrapping
    | Viewport({wrapping, _}) => wrapping
    | WrapColumn({wrapping, _}) => wrapping
    | Bounded({wrapping, _}) => wrapping;

  // TODO: Bring back when wrapping strategy is hooked up!
  //  let _resize = (~columns: int, ~buffer, wrapState) => {
  //    switch (wrapState) {
  //    // All the cases where we don't need to update wrapping...
  //    | NoWrap(_) as nowrap => nowrap
  //    | WrapColumn(_) as wrapcolumn => wrapcolumn
  //    | Viewport({lastWrapColumn, _}) as viewport when lastWrapColumn == columns => viewport
  //    | Bounded({lastWrapColumn, _}) as bounded when lastWrapColumn == columns => bounded
  //    // And the cases where we may need to update wrapping
  //    | Viewport(_) =>
  //      // TODO:
  //      //let wrapping = Wrapping.make(~wrap=WordWrap.fixed(~columns), ~buffer);
  //      let wrapping = Wrapping.make(~wrap=WordWrap.none, ~buffer);
  //      Viewport({lastWrapColumn: columns, wrapping});
  //    | Bounded({wrapColumn, lastWrapColumn, _}) as bounded =>
  //      let newWrapColumn = min(wrapColumn, columns);
  //      if (newWrapColumn == lastWrapColumn) {
  //        bounded;
  //      } else {
  //        // TODO:
  //        //        let wrapping =
  //        //          Wrapping.make(
  //        //            ~wrap=WordWrap.fixed(~columns=newWrapColumn),
  //        //            ~buffer,
  //        //          );
  //        let wrapping = Wrapping.make(~wrap=WordWrap.none, ~buffer);
  //        Bounded({wrapColumn, lastWrapColumn: newWrapColumn, wrapping});
  //      };
  //    };
  //  };

  let map = f =>
    fun
    | NoWrap({wrapping}) => NoWrap({wrapping: f(wrapping)})
    | Viewport({wrapping, lastWrapColumn}) =>
      Viewport({wrapping: f(wrapping), lastWrapColumn})
    | WrapColumn({wrapping, wrapColumn}) =>
      WrapColumn({wrapping: f(wrapping), wrapColumn})
    | Bounded({wrapping, wrapColumn, lastWrapColumn}) =>
      Bounded({wrapping: f(wrapping), wrapColumn, lastWrapColumn});

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
  lineHeight: LineHeight.t,
  scrollX: float,
  scrollY: float,
  isScrollAnimated: bool,
  isMinimapEnabled: bool,
  minimapMaxColumnWidth: int,
  minimapScrollY: float,
  cursors: [@opaque] list(BytePosition.t),
  selection: [@opaque] option(VisualRange.t),
  pixelWidth: int,
  pixelHeight: int,
  yankHighlight: option(yankHighlight),
  wrapMode: WrapMode.t,
  wrapState: WrapState.t,
};

let key = ({key, _}) => key;
let selection = ({selection, _}) => selection;
let setSelection = (~selection, editor) => {
  ...editor,
  selection: Some(selection),
};
let clearSelection = editor => {...editor, selection: None};
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

let setMinimapEnabled = (~enabled, editor) => {
  ...editor,
  isMinimapEnabled: enabled,
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

  let viewEndByte =
    BufferLine.Slow.getByteFromPixel(
      ~relativeToByte=viewStartByte,
      ~pixelX=float(editor.pixelWidth),
      bufferLine,
    );

  let viewStartIndex = BufferLine.getIndex(~byte=viewStartByte, bufferLine);
  let viewEndIndex = BufferLine.getIndex(~byte=viewEndByte, bufferLine);

  BufferViewTokenizer.tokenize(
    ~start=viewStartIndex,
    ~stop=CharacterIndex.(viewEndIndex + 1),
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

let create = (~wrapMode=WrapMode.NoWrap, ~config, ~buffer, ()) => {
  let id = GlobalState.generateId();
  let key = Brisk_reconciler.Key.create();

  let isMinimapEnabled = EditorConfiguration.Minimap.enabled.get(config);
  let lineHeight = EditorConfiguration.lineHeight.get(config);

  let wrapState = WrapState.make(~wrapMode, ~buffer);

  {
    editorId: id,
    key,
    lineHeight,
    isMinimapEnabled,
    isScrollAnimated: false,
    buffer,
    scrollX: 0.,
    scrollY: 0.,
    minimapMaxColumnWidth: Constants.minimapMaxColumn,
    minimapScrollY: 0.,
    /*
     * We need an initial editor size, otherwise we'll immediately scroll the view
     * if a buffer loads prior to our first render.
     */
    cursors: [{line: EditorCoreTypes.LineNumber.zero, byte: ByteIndex.zero}],
    selection: None,
    pixelWidth: 1,
    pixelHeight: 1,
    yankHighlight: None,
    wrapState,
    wrapMode,
  };
};

let totalViewLines = ({wrapState, _}) =>
  wrapState |> WrapState.wrapping |> Wrapping.numberOfLines;
let maxLineLength = ({wrapState, _}) =>
  wrapState |> WrapState.wrapping |> Wrapping.maxLineLength;

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

let getCursors = ({cursors, _}) => cursors;

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

let getCharacterBehindCursor = ({cursors, buffer, _}) => {
  switch (cursors) {
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

let getCharacterUnderCursor = ({cursors, buffer, _}) => {
  switch (cursors) {
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
    switch (editor.cursors) {
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
  switch (editor.cursors) {
  | [cursor, ..._] => cursor
  | [] =>
    BytePosition.{line: EditorCoreTypes.LineNumber.zero, byte: ByteIndex.zero}
  };
let selectionOrCursorRange = editor => {
  switch (editor.selection) {
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

let getLayout = (~showLineNumbers, ~maxMinimapCharacters, view) => {
  let {pixelWidth, pixelHeight, isMinimapEnabled, _} = view;
  let layout: EditorLayout.t =
    EditorLayout.getLayout(
      ~showLineNumbers,
      ~isMinimapShown=isMinimapEnabled,
      ~maxMinimapCharacters,
      ~pixelWidth=float_of_int(pixelWidth),
      ~pixelHeight=float_of_int(pixelHeight),
      ~characterWidth=getCharacterWidth(view),
      ~characterHeight=lineHeightInPixels(view),
      ~bufferLineCount=view |> totalViewLines,
      (),
    );

  layout;
};

let exposePrimaryCursor = editor => {
  switch (editor.cursors) {
  | [primaryCursor, ..._tail] =>
    let {bufferWidthInPixels, _}: EditorLayout.t =
      getLayout(~showLineNumbers=true, ~maxMinimapCharacters=999, editor);

    let pixelWidth = bufferWidthInPixels;

    let {pixelHeight, scrollX, scrollY, _} = editor;
    let pixelHeight = float(pixelHeight);

    let ({x: pixelX, y: pixelY}: PixelPosition.t, _width) =
      bufferBytePositionToPixel(~position=primaryCursor, editor);

    let scrollOffX = getCharacterWidth(editor) *. 2.;
    let scrollOffY = lineHeightInPixels(editor);

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
      if (pixelY < 0.) {
        scrollY +. pixelY;
      } else if (pixelY >= availableY) {
        scrollY +. (pixelY -. availableY);
      } else {
        scrollY;
      };

    {...editor, scrollX: adjustedScrollX, scrollY: adjustedScrollY};

  | _ => editor
  };
};

let setCursors = (~cursors, editor) =>
  {...editor, cursors} |> exposePrimaryCursor;

let getLeftVisibleColumn = view => {
  int_of_float(view.scrollX /. getCharacterWidth(view));
};

let getTopVisibleLine = view =>
  int_of_float(view.scrollY /. lineHeightInPixels(view)) + 1;

let getBottomVisibleLine = view => {
  let absoluteBottomLine =
    int_of_float(
      (view.scrollY +. float_of_int(view.pixelHeight))
      /. lineHeightInPixels(view),
    );

  let viewLines = view |> totalViewLines;

  absoluteBottomLine > viewLines ? viewLines : absoluteBottomLine;
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

let setSize = (~pixelWidth, ~pixelHeight, editor) => {
  ...editor,
  pixelWidth,
  pixelHeight,
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

let scrollToColumn = (~column, view) => {
  let pixelX = float_of_int(column) *. getCharacterWidth(view);
  {...scrollToPixelX(~pixelX, view), isScrollAnimated: true};
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
    wrapState: WrapState.make(~wrapMode=editor.wrapMode, ~buffer),
  };
};

module Slow = {
  let pixelPositionToBytePosition =
      (~allowPast=false, ~buffer, ~pixelX: float, ~pixelY: float, view) => {
    let rawLine =
      int_of_float((pixelY +. view.scrollY) /. lineHeightInPixels(view));

    let totalLinesInBuffer = Buffer.getNumberOfLines(buffer);

    let lineIdx =
      if (rawLine >= totalLinesInBuffer) {
        max(0, totalLinesInBuffer - 1);
      } else {
        rawLine;
      };

    if (lineIdx >= 0 && lineIdx < totalLinesInBuffer) {
      let bufferLine = Buffer.getLine(lineIdx, buffer);
      let index =
        BufferLine.Slow.getIndexFromPixel(
          ~pixel=pixelX +. view.scrollX,
          bufferLine,
        );

      let byteIndex = BufferLine.getByteFromIndex(~index, bufferLine);

      let bytePositionInBounds =
        BytePosition.{
          line: EditorCoreTypes.LineNumber.ofZeroBased(lineIdx),
          byte: byteIndex,
        };
      if (allowPast
          && ByteIndex.toInt(byteIndex)
          == BufferLine.lengthInBytes(bufferLine)
          - 1) {
        // If we're allowed to return a byte index _after_ the length of the line - like for insert mode
        // Check if we actually exceeded the bounds

        let (cursorOffset, width) =
          BufferLine.getPixelPositionAndWidth(~index, bufferLine);

        if (cursorOffset +. width < pixelX) {
          BytePosition.{
            line: bytePositionInBounds.line,
            byte: ByteIndex.(byteIndex + 1),
          };
        } else {
          bytePositionInBounds;
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
