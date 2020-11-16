open EditorCoreTypes;
open Oni_Core;
open Utility;
open Component_Animation;

module GlobalState = {
  let lastId = ref(0);

  let generateId = () => {
    let id = lastId^;
    incr(lastId);
    id;
  };
};

type inlineElement = {
  reconcilerKey: Brisk_reconciler.Key.t,
  hidden: bool,
  key: string,
  uniqueId: string,
  lineNumber: EditorCoreTypes.LineNumber.t,
  view:
    (~theme: Oni_Core.ColorTheme.Colors.t, ~uiFont: UiFont.t, unit) =>
    Revery.UI.element,
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
  opacity: [@opaque] Component_Animation.t(float),
};

let scrollSpringOptions =
  Revery.UI.Spring.Options.create(~stiffness=310., ~damping=30., ());
[@deriving show]
type t = {
  key: [@opaque] Brisk_reconciler.Key.t,
  buffer: [@opaque] EditorBuffer.t,
  editorId: EditorId.t,
  lineNumbers: [ | `Off | `On | `Relative | `RelativeOnly],
  lineHeight: LineHeight.t,
  scrollX: [@opaque] Component_Animation.Spring.t,
  scrollY: [@opaque] Component_Animation.Spring.t,
  inlineElements: InlineElements.t,
  isScrollAnimated: bool,
  isMinimapEnabled: bool,
  minimapMaxColumnWidth: int,
  minimapScrollY: float,
  mode: [@opaque] Vim.Mode.t,
  pixelWidth: int,
  pixelHeight: int,
  yankHighlight: option(yankHighlight),
  yankHighlightDuration: int,
  wrapMode: WrapMode.t,
  wrapState: WrapState.t,
  wrapPadding: option(float),
  // Number of lines to preserve before or after the cursor, when scrolling.
  // Like the `scrolloff` vim setting or the `editor.cursorSurroundingLines` VSCode setting.
  verticalScrollMargin: int,
  // Mouse state
  isMouseDown: bool,
  hasMouseEntered: bool,
  // The last mouse position, in screen coordinates
  lastMouseScreenPosition: option(PixelPosition.t),
  lastMouseMoveTime: [@opaque] option(Revery.Time.t),
  lastMouseUpTime: [@opaque] option(Revery.Time.t),
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
let scrollY = ({scrollY, _}) => Spring.get(scrollY);
let scrollX = ({scrollX, _}) => Spring.get(scrollX);
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

let bufferBytePositionToPixelInternal =
    (
      ~useAnimatedScrollPosition: bool,
      ~position: BytePosition.t,
      {scrollX, scrollY, buffer, wrapState, _} as editor,
    ) => {
  let (scrollX, scrollY) =
    if (useAnimatedScrollPosition) {
      (Spring.get(scrollX), Spring.get(scrollY));
    } else {
      (Spring.getTarget(scrollX), Spring.getTarget(scrollY));
    };
  let lineCount = EditorBuffer.numberOfLines(buffer);
  let line = position.line |> EditorCoreTypes.LineNumber.toZeroBased;
  if (line < 0 || line >= lineCount) {
    ({x: 0., y: 0.}: PixelPosition.t, 0.);
  } else {
    let wrapping = wrapState |> WrapState.wrapping;
    let bufferLine = buffer |> EditorBuffer.line(line);

    let viewLine =
      Wrapping.bufferBytePositionToViewLine(~bytePosition=position, wrapping);

    let {line: bufferLineNum, characterOffset: viewStartIndex, _}: Wrapping.bufferPosition =
      Wrapping.viewLineToBufferPosition(~line=viewLine, wrapping);

    let (startPixel, _width) =
      BufferLine.getPixelPositionAndWidth(~index=viewStartIndex, bufferLine);

    let index = BufferLine.getIndex(~byte=position.byte, bufferLine);
    let (actualPixel, width) =
      BufferLine.getPixelPositionAndWidth(~index, bufferLine);

    // Account for inline elements, like codelens
    let inlineElementOffsetY =
      InlineElements.getReservedSpace(bufferLineNum, editor.inlineElements);

    let pixelX = actualPixel -. startPixel -. scrollX +. 0.5;
    let pixelY =
      inlineElementOffsetY
      +. lineHeightInPixels(editor)
      *. float(viewLine)
      -. scrollY
      +. 0.5;

    ({x: pixelX, y: pixelY}: PixelPosition.t, width);
  };
};

let bufferBytePositionToPixel =
  bufferBytePositionToPixelInternal(~useAnimatedScrollPosition=true);

module Animations = {
  open Revery.UI;
  let fadeIn = (~duration) =>
    Revery.UI.Animation.(
      animate(Revery.Time.milliseconds(duration))
      |> ease(Easing.easeIn)
      |> tween(0.6, 0.0)
      |> delay(Revery.Time.milliseconds(0))
    );
};

let yankHighlight = ({yankHighlight, _}) => yankHighlight;
let startYankHighlight = (pixelRanges, editor) => {
  ...editor,
  yankHighlight:
    Some({
      pixelRanges,
      key: Brisk_reconciler.Key.create(),
      opacity:
        Component_Animation.make(
          Animations.fadeIn(~duration=editor.yankHighlightDuration),
        ),
    }),
};

let setWrapPadding = (~padding, editor) => {
  ...editor,
  wrapPadding: Some(padding),
};

let setVerticalScrollMargin = (~lines, editor) => {
  ...editor,
  verticalScrollMargin: lines,
};

let totalViewLines = ({wrapState, _}) =>
  wrapState |> WrapState.wrapping |> Wrapping.numberOfLines;

let getCharacterWidth = ({buffer, _}) =>
  EditorBuffer.font(buffer).spaceWidth;

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

  let tokens =
    BufferViewTokenizer.tokenize(
      ~start=viewStartIndex,
      ~stop=viewEndIndex,
      bufferLine,
      colorizer(~startByte=viewStartByte),
    );

  // The tokens returned from tokenize start at a pixel position of 0.
  // However, there may be a scroll applied, and we need to account for
  // shifting the tokens if the scroll position is not exactly aligned to
  // a character.
  let ({x: startIndexPixel, _}: PixelPosition.t, _width) = {
    let editorWithScrollApplied = {
      ...editor,
      scrollX: Spring.set(~instant=true, ~position=scrollX, editor.scrollX),
    };
    bufferBytePositionToPixel(
      ~position=BytePosition.{line: bufferPosition.line, byte: viewStartByte},
      editorWithScrollApplied,
    );
  };

  let pixelOffset = startIndexPixel;

  tokens
  |> List.map((token: BufferViewTokenizer.t) =>
       {...token, startPixel: token.startPixel +. pixelOffset}
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

let getContentPixelWidth = editor => {
  let layout: EditorLayout.t = getLayout(editor);
  layout.bufferWidthInPixels;
};

let setLineHeight = (~lineHeight, editor) => {...editor, lineHeight};

let setLineNumbers = (~lineNumbers, editor) => {...editor, lineNumbers};
let lineNumbers = ({lineNumbers, _}) => lineNumbers;

let setWrapMode = (~wrapMode, editor) => {
  let pixelWidth = getContentPixelWidth(editor);
  {
    ...editor,
    wrapMode,
    wrapState: WrapState.make(~pixelWidth, ~wrapMode, ~buffer=editor.buffer),
  };
};

let configure = (~config, editor) => {
  let wrapMode =
    EditorConfiguration.Experimental.wordWrap.get(config) == `On
      ? WrapMode.Viewport : WrapMode.NoWrap;

  let scrolloff = EditorConfiguration.scrolloff.get(config);

  let isScrollAnimated =
    Feature_Configuration.GlobalConfiguration.animation.get(config)
    && EditorConfiguration.smoothScroll.get(config);

  let yankHighlightDuration =
    EditorConfiguration.yankHighlightDuration.get(config);

  {...editor, isScrollAnimated, yankHighlightDuration}
  |> setVerticalScrollMargin(~lines=scrolloff)
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

let create = (~config, ~buffer, ()) => {
  let id = GlobalState.generateId();
  let key = Brisk_reconciler.Key.create();

  let wrapMode =
    EditorConfiguration.Experimental.wordWrap.get(config) == `On
      ? WrapMode.Viewport : WrapMode.NoWrap;

  let wrapState = WrapState.make(~pixelWidth=1000., ~wrapMode, ~buffer);

  let isScrollAnimated =
    Feature_Configuration.GlobalConfiguration.animation.get(config)
    && EditorConfiguration.smoothScroll.get(config);

  let yankHighlightDuration =
    EditorConfiguration.yankHighlightDuration.get(config);

  {
    editorId: id,
    key,
    lineHeight: LineHeight.default,
    lineNumbers: `On,
    isMinimapEnabled: true,
    isScrollAnimated,
    buffer,
    scrollX: Spring.make(~restThreshold=1., ~options=scrollSpringOptions, 0.),
    scrollY: Spring.make(~restThreshold=1., ~options=scrollSpringOptions, 0.),
    minimapMaxColumnWidth: 99,
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
    yankHighlightDuration,
    wrapState,
    wrapMode,
    wrapPadding: None,
    verticalScrollMargin: 1,
    inlineElements: InlineElements.initial,
    isMouseDown: false,
    hasMouseEntered: false,
    lastMouseMoveTime: None,
    lastMouseScreenPosition: None,
    lastMouseUpTime: None,
  }
  |> configure(~config);
};

let cursors = ({mode, _}) => Vim.Mode.cursors(mode);

let maxLineLength = ({wrapState, _}) =>
  wrapState |> WrapState.wrapping |> Wrapping.maxLineLength;

let viewLineToPixelY = (idx, editor) => {
  let wrapping = editor.wrapState |> WrapState.wrapping;
  let {line: bufferLine, _}: Wrapping.bufferPosition =
    Wrapping.viewLineToBufferPosition(~line=idx, wrapping);
  let inlineElementOffsetY =
    InlineElements.getReservedSpace(bufferLine, editor.inlineElements);
  inlineElementOffsetY +. lineHeightInPixels(editor) *. float(idx);
};

let getViewLineFromPixelY = (~pixelY, editor) => {
  let lineHeight = lineHeightInPixels(editor);
  let wrapping = editor.wrapState |> WrapState.wrapping;
  let rec loop =
          (accumulatedLines, accumulatedPixels, remainingInlineElements) =>
    if (pixelY < accumulatedPixels) {
      accumulatedLines - 1;
    } else {
      switch ((remainingInlineElements: list(InlineElements.element))) {
      | [] =>
        let lineNumber =
          int_of_float((pixelY -. accumulatedPixels) /. lineHeight);
        lineNumber + accumulatedLines;
      | [hd, ...tail] =>
        let viewLine =
          Wrapping.bufferBytePositionToViewLine(
            ~bytePosition=BytePosition.{line: hd.line, byte: ByteIndex.zero},
            wrapping,
          );
        let additionalRegion =
          float(viewLine - accumulatedLines) *. lineHeight;
        if (additionalRegion +. accumulatedPixels >= pixelY) {
          // The line is prior to the next inline element!
          let lineNumber =
            int_of_float((pixelY -. accumulatedPixels) /. lineHeight);
          lineNumber + accumulatedLines;
        } else {
          // We need to advance
          loop(
            viewLine + 1,
            accumulatedPixels +. additionalRegion +. hd.height +. lineHeight,
            tail,
          );
        };
      };
    };

  loop(0, 0., editor.inlineElements |> InlineElements.allElements);
};

let getTopViewLine = editor => {
  getViewLineFromPixelY(~pixelY=Spring.get(editor.scrollY), editor);
};

let getTopVisibleBufferLine = editor => {
  let topViewLine = getTopViewLine(editor);
  viewLineToBufferLine(topViewLine, editor);
};

let getBottomViewLine = editor => {
  let absoluteBottomLine =
    getViewLineFromPixelY(
      ~pixelY=Spring.get(editor.scrollY) +. float_of_int(editor.pixelHeight),
      editor,
    );

  let viewLines = editor |> totalViewLines;

  absoluteBottomLine >= viewLines ? viewLines - 1 : absoluteBottomLine;
};

let getBottomVisibleBufferLine = editor => {
  let viewBottomLine = getBottomViewLine(editor);
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

let characterRangeToByteRange = ({start, stop}: CharacterRange.t, editor) => {
  let maybeByteStart = characterToByte(start, editor);
  let maybeByteStop = characterToByte(stop, editor);

  Oni_Core.Utility.OptionEx.map2(
    (start, stop) => {ByteRange.{start, stop}},
    maybeByteStart,
    maybeByteStop,
  );
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

let withSteadyCursor = (f, editor) => {
  let bytePosition = getPrimaryCursorByte(editor);

  let calculateOffset = (bytePosition, editor) => {
    let wrapping = editor.wrapState |> WrapState.wrapping;
    let viewLine =
      Wrapping.bufferBytePositionToViewLine(~bytePosition, wrapping);

    viewLineToPixelY(viewLine, editor);
  };

  let originalOffset = calculateOffset(bytePosition, editor);
  let editor' = f(editor);

  let newOffset = calculateOffset(bytePosition, editor');
  let scrollYValue =
    Spring.getTarget(editor.scrollY) +. (newOffset -. originalOffset);
  let scrollY =
    Spring.set(~instant=true, ~position=scrollYValue, editor.scrollY);
  {...editor', scrollY};
};

let makeInlineElement = (~key, ~uniqueId, ~lineNumber, ~view) => {
  hidden: false,
  reconcilerKey: Brisk_reconciler.Key.create(),
  key,
  uniqueId,
  lineNumber,
  view,
};

let setInlineElementSize = (~key, ~uniqueId, ~height, editor) => {
  editor
  |> withSteadyCursor(e =>
       {
         ...e,
         inlineElements:
           InlineElements.setSize(
             ~key,
             ~uniqueId,
             ~height=float(height),
             e.inlineElements,
           ),
       }
     );
};

let setInlineElements = (~key, ~elements: list(inlineElement), editor) => {
  let elements': list(InlineElements.element) =
    elements
    |> List.map((inlineElement: inlineElement) =>
         InlineElements.{
           reconcilerKey: Brisk_reconciler.Key.create(),
           key: inlineElement.key,
           uniqueId: inlineElement.uniqueId,
           line: inlineElement.lineNumber,
           height: 0.,
           view: inlineElement.view,
           hidden: inlineElement.hidden,
         }
       );

  editor
  |> withSteadyCursor(e =>
       {
         ...e,
         inlineElements:
           InlineElements.set(~key, ~elements=elements', e.inlineElements),
       }
     );
};

let getInlineElements = ({inlineElements, _}) => {
  inlineElements |> InlineElements.allElements;
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

let getId = model => model.editorId;

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

  let topF = Spring.get(view.scrollY) /. totalViewSizeInPixels;
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

      let topF = Spring.get(editor.scrollX) /. totalViewWidthInPixels;
      let thumbOffset = int_of_float(topF *. availableWidthF);

      {thumbSize, thumbOffset, visible: true};
    };
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

let hasSetSize = editor => {
  editor.pixelWidth > 1 && editor.pixelHeight > 1;
};

let exposePrimaryCursor = editor =>
  if (!hasSetSize(editor)) {
    // If the size hasn't been set yet - don't try to expose the cursor.
    // We don't know about the viewport to do a good job.
    // See:
    editor;
  } else {
    switch (cursors(editor)) {
    | [primaryCursor, ..._tail] =>
      let {bufferWidthInPixels, _}: EditorLayout.t = getLayout(editor);

      let pixelWidth = bufferWidthInPixels;

      let {pixelHeight, scrollX, scrollY, _} = editor;
      let pixelHeight = float(pixelHeight);

      let ({x: pixelX, y: pixelY}: PixelPosition.t, _width) =
        bufferBytePositionToPixelInternal(
          ~useAnimatedScrollPosition=false,
          ~position=primaryCursor,
          editor,
        );

      let scrollOffX = getCharacterWidth(editor) *. 2.;
      let scrollOffY =
        lineHeightInPixels(editor)
        *. float(max(editor.verticalScrollMargin, 0));

      let availableX = pixelWidth -. scrollOffX;
      let availableY = pixelHeight -. scrollOffY;

      let scrollX = Spring.getTarget(scrollX);
      let adjustedScrollX =
        max(
          if (pixelX < 0.) {
            scrollX +. pixelX;
          } else if (pixelX >= availableX) {
            scrollX +. (pixelX -. availableX);
          } else {
            scrollX;
          },
          0.,
        );

      let scrollY = Spring.getTarget(scrollY);
      let adjustedScrollY =
        max(
          if (pixelY < scrollOffY) {
            scrollY -. scrollOffY +. pixelY;
          } else if (pixelY >= availableY) {
            scrollY +. (pixelY -. availableY +. lineHeightInPixels(editor));
          } else {
            scrollY;
          },
          0.,
        );

      let animated = editor.isScrollAnimated;
      {
        ...editor,
        scrollX:
          Spring.set(
            ~instant=!animated,
            ~position=adjustedScrollX,
            editor.scrollX,
          ),
        scrollY:
          Spring.set(
            ~instant=!animated,
            ~position=adjustedScrollY,
            editor.scrollY,
          ),
      };

    | _ => editor
    };
  };

let mode = ({mode, _}) => mode;

let setMode = (mode, editor) => {
  {...editor, mode} |> exposePrimaryCursor;
};

let getLeftVisibleColumn = view => {
  int_of_float(Spring.get(view.scrollX) /. getCharacterWidth(view));
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

let scrollToPixelY = (~animated, ~pixelY as newScrollY, editor) => {
  let animated = editor.isScrollAnimated && animated;
  let {pixelHeight, _} = editor;
  let viewLines = editor |> totalViewLines;
  let newScrollY = max(0., newScrollY);
  let availableScroll =
    max(float_of_int(viewLines - 1), 0.)
    *. lineHeightInPixels(editor)
    +. InlineElements.getAllReservedSpace(editor.inlineElements);
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
    minimapScrollY: newMinimapScroll,
    scrollY:
      Spring.set(~instant=!animated, ~position=newScrollY, editor.scrollY),
  };
};

//let animateScroll = editor => {...editor, isScrollAnimated: true};

let scrollToLine = (~line, view) => {
  let pixelY = float_of_int(line) *. lineHeightInPixels(view);
  scrollToPixelY(~animated=true, ~pixelY, view);
};

let scrollToPixelX = (~animated, ~pixelX as newScrollX, editor) => {
  let animated = editor.isScrollAnimated && animated;
  let maxLineLength = editor |> maxLineLength;
  let newScrollX = max(0., newScrollX);

  let availableScroll =
    max(0., float_of_int(maxLineLength) *. getCharacterWidth(editor));
  let newScrollX = min(newScrollX, availableScroll);
  let scrollX =
    Spring.set(~instant=!animated, ~position=newScrollX, editor.scrollX);

  {...editor, scrollX};
};

let scrollDeltaPixelX = (~animated, ~pixelX, editor) => {
  let pixelX = Spring.getTarget(editor.scrollX) +. pixelX;
  scrollToPixelX(~animated, ~pixelX, editor);
};

let scrollDeltaPixelY = (~animated, ~pixelY, editor) => {
  let pixelY = Spring.getTarget(editor.scrollY) +. pixelY;
  scrollToPixelY(~animated, ~pixelY, editor);
};

let scrollToPixelXY =
    (~animated, ~pixelX as newScrollX, ~pixelY as newScrollY, view) => {
  let {scrollX, _} = scrollToPixelX(~animated, ~pixelX=newScrollX, view);
  let {scrollY, minimapScrollY, _} =
    scrollToPixelY(~animated, ~pixelY=newScrollY, view);

  {...view, scrollX, scrollY, minimapScrollY};
};

let scrollDeltaPixelXY = (~animated, ~pixelX, ~pixelY, view) => {
  let {scrollX, _} = scrollDeltaPixelX(~animated, ~pixelX, view);
  let {scrollY, minimapScrollY, _} =
    scrollDeltaPixelY(~animated, ~pixelY, view);

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
    bufferBytePositionToPixelInternal(
      ~useAnimatedScrollPosition=false,
      ~position=oldCursor,
      editor,
    );

  let ({y: newY, _}: PixelPosition.t, _) =
    bufferBytePositionToPixelInternal(
      ~useAnimatedScrollPosition=false,
      ~position=newCursor,
      editor',
    );

  // Adjust scroll to compensate cursor position - keeping the cursor in the same
  // relative spot to scroll, after moving it.
  // TODO: Bring back
  editor' |> scrollDeltaPixelY(~animated=true, ~pixelY=newY -. oldY);
};

let scrollCenterCursorVertically = editor => {
  let cursor = getPrimaryCursorByte(editor);
  let (pixelPosition: PixelPosition.t, _) =
    bufferBytePositionToPixelInternal(
      ~useAnimatedScrollPosition=false,
      ~position=cursor,
      editor,
    );

  let heightInPixels = float(editor.pixelHeight);
  let pixelY =
    pixelPosition.y
    +. Spring.getTarget(editor.scrollY)
    -. heightInPixels
    /. 2.;
  scrollToPixelY(~animated=true, ~pixelY, editor);
};

let scrollCursorTop = editor => {
  let cursor = getPrimaryCursorByte(editor);
  let (pixelPosition: PixelPosition.t, _) =
    bufferBytePositionToPixelInternal(
      ~useAnimatedScrollPosition=false,
      ~position=cursor,
      editor,
    );

  let pixelY = pixelPosition.y +. Spring.getTarget(editor.scrollY);
  editor |> scrollToPixelY(~animated=true, ~pixelY) |> exposePrimaryCursor; // account for scrolloff
};

let scrollCursorBottom = editor => {
  let cursor = getPrimaryCursorByte(editor);
  let (pixelPosition: PixelPosition.t, _) =
    bufferBytePositionToPixelInternal(
      ~useAnimatedScrollPosition=false,
      ~position=cursor,
      editor,
    );

  let heightInPixels = float(editor.pixelHeight);
  let pixelY =
    pixelPosition.y
    +. Spring.getTarget(editor.scrollY)
    -. (heightInPixels -. lineHeightInPixels(editor));
  scrollToPixelY(~animated=true, ~pixelY, editor) |> exposePrimaryCursor; // account for scrolloff
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

let setSize = (~pixelWidth, ~pixelHeight, originalEditor) => {
  let editor = {...originalEditor, pixelWidth, pixelHeight};

  let contentPixelWidth = getContentPixelWidth(editor);

  let wrapPadding =
    switch (editor.wrapPadding) {
    | None => EditorBuffer.measure(Uchar.of_char('W'), editor.buffer)
    | Some(padding) => padding
    };

  let wrapWidth = contentPixelWidth -. wrapPadding;
  let wrapState =
    WrapState.resize(
      ~pixelWidth=wrapWidth,
      ~buffer=editor.buffer,
      editor.wrapState,
    );

  let editor' = {...editor, wrapState};

  // If we hadn't measured before, make sure the cursor is in view
  if (!hasSetSize(originalEditor)) {
    scrollCursorTop(editor');
  } else {
    editor';
  };
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
  editor
  |> withSteadyCursor(editor =>
       {
         ...editor,
         buffer,
         wrapState: WrapState.update(~update, ~buffer, editor.wrapState),
         inlineElements: InlineElements.shift(update, editor.inlineElements),
       }
     );
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

let configurationChanged = (~perFileTypeConfig, editor) => {
  let fileType =
    editor.buffer |> EditorBuffer.fileType |> Oni_Core.Buffer.FileType.toString;

  let config = perFileTypeConfig(~fileType);
  configure(~config, editor);
};

module Slow = {
  let pixelPositionToBytePosition =
      (~allowPast=false, ~pixelX: float, ~pixelY: float, view) => {
    let rawLine =
      getViewLineFromPixelY(
        ~pixelY=pixelY +. Spring.get(view.scrollY),
        view,
      );

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

    if (lineIdx >= 0 && lineIdx < totalLinesInBuffer) {
      let bufferLine = EditorBuffer.line(lineIdx, view.buffer);
      let byteIndex =
        BufferLine.Slow.getByteFromPixel(
          ~relativeToByte=byteOffset,
          ~pixelX=pixelX +. Spring.get(view.scrollX),
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

let mouseDown = (~time, ~pixelX, ~pixelY, editor) => {
  ignore(time);
  ignore(pixelX);
  ignore(pixelY);
  {...editor, isMouseDown: true};
};

let getCharacterUnderMouse = editor => {
  editor.lastMouseScreenPosition
  |> OptionEx.flatMap((mousePosition: PixelPosition.t) => {
       let bytePosition: BytePosition.t =
         Slow.pixelPositionToBytePosition(
           ~allowPast=true,
           ~pixelX=mousePosition.x,
           ~pixelY=mousePosition.y,
           editor,
         );

       let bufferLine =
         EditorBuffer.line(
           EditorCoreTypes.LineNumber.toZeroBased(bytePosition.line),
           editor.buffer,
         );
       if (BufferLine.lengthInBytes(bufferLine)
           > ByteIndex.toInt(bytePosition.byte)) {
         byteToCharacter(bytePosition, editor);
       } else {
         None;
       };
     });
};

let mouseUp = (~time, ~pixelX, ~pixelY, editor) => {
  let isDoubleClick =
    switch (editor.lastMouseUpTime) {
    | Some(lastMouseUpTime) =>
      let lastTime = Revery.Time.toFloatSeconds(lastMouseUpTime);
      let newTime = Revery.Time.toFloatSeconds(time);
      let doubleClickTime =
        Revery.Time.toFloatSeconds(Constants.doubleClickTime);
      newTime -. lastTime < doubleClickTime;
    | None => false
    };

  let isInsertMode = Vim.Mode.isInsert(editor.mode);
  if (isDoubleClick) {
    let mode =
      getCharacterUnderMouse(editor)
      |> OptionEx.flatMap(characterPosition => {
           getTokenAt(
             ~languageConfiguration=LanguageConfiguration.default,
             characterPosition,
             editor,
           )
         })
      |> OptionEx.flatMap(characterRange =>
           characterRangeToByteRange(characterRange, editor)
         )
      |> Option.map((byteRange: ByteRange.t) => {
           let visualRange =
             Vim.VisualRange.{
               cursor: byteRange.start,
               anchor: byteRange.stop,
               visualType: Vim.Types.Character,
             };

           if (isInsertMode) {
             Vim.Mode.Select(visualRange);
           } else {
             Vim.Mode.Visual(visualRange);
           };
         })
      |> Option.value(~default=editor.mode);

    {...editor, isMouseDown: false, mode, lastMouseUpTime: None};
  } else {
    let bytePosition =
      Slow.pixelPositionToBytePosition(
        // #2463: When we're insert mode, clicking past the end of the line
        // should move the cursor past the last byte
        ~allowPast=isInsertMode,
        ~pixelX,
        ~pixelY,
        editor,
      );
    let mode =
      if (Vim.Mode.isInsert(editor.mode)) {
        Vim.Mode.Insert({cursors: [bytePosition]});
      } else {
        Vim.Mode.Normal({cursor: bytePosition});
      };
    {...editor, isMouseDown: false, mode, lastMouseUpTime: Some(time)};
  };
};

let mouseMove = (~time, ~pixelX, ~pixelY, editor) => {
  {
    ...editor,
    lastMouseMoveTime: Some(time),
    lastMouseScreenPosition: Some(PixelPosition.{x: pixelX, y: pixelY}),
  };
};

let mouseEnter = editor => {
  ...editor,
  hasMouseEntered: true,
  lastMouseMoveTime: None,
  lastMouseScreenPosition: None,
};
let mouseLeave = editor => {
  ...editor,
  hasMouseEntered: false,
  isMouseDown: false,
  lastMouseMoveTime: None,
  lastMouseScreenPosition: None,
};

let hasMouseEntered = ({hasMouseEntered, _}) => hasMouseEntered;

let isMouseDown = ({isMouseDown, _}) => isMouseDown;

let lastMouseMoveTime = ({lastMouseMoveTime, _}) => lastMouseMoveTime;

let getLeadingWhitespacePixels = (lineNumber, editor) => {
  let buffer = editor.buffer;
  let lineCount = EditorBuffer.numberOfLines(buffer);
  let line = lineNumber |> EditorCoreTypes.LineNumber.toZeroBased;
  if (line < 0 || line >= lineCount) {
    0.;
  } else {
    let bufferLine = buffer |> EditorBuffer.line(line);
    BufferLine.getLeadingWhitespacePixels(bufferLine);
  };
};

[@deriving show]
type msg =
  | ScrollSpringX([@opaque] Spring.msg)
  | ScrollSpringY([@opaque] Spring.msg)
  | YankHighlight([@opaque] Component_Animation.msg);

let update = (msg, editor) => {
  switch (msg) {
  | YankHighlight(msg) =>
    let yankHighlight' =
      yankHighlight(editor)
      |> OptionEx.flatMap(yankHighlight => {
           let opacity' =
             Component_Animation.update(msg, yankHighlight.opacity);

           if (Component_Animation.isComplete(opacity')) {
             None;
           } else {
             Some({...yankHighlight, opacity: opacity'});
           };
         });
    {...editor, yankHighlight: yankHighlight'};
  | ScrollSpringX(msg) => {
      ...editor,
      scrollX: Spring.update(msg, editor.scrollX),
    }
  | ScrollSpringY(msg) => {
      ...editor,
      scrollY: Spring.update(msg, editor.scrollY),
    }
  };
};

let sub = editor => {
  let yankHighlightAnimation =
    yankHighlight(editor)
    |> Option.map(({opacity, _}) => {
         opacity
         |> Component_Animation.sub
         |> Isolinear.Sub.map(msg => YankHighlight(msg))
       })
    |> Option.value(~default=Isolinear.Sub.none);
  [
    Spring.sub(editor.scrollX) |> Isolinear.Sub.map(msg => ScrollSpringX(msg)),
    Spring.sub(editor.scrollY)
    |> Isolinear.Sub.map(msg => ScrollSpringY(msg)),
    yankHighlightAnimation,
  ]
  |> Isolinear.Sub.batch;
};
