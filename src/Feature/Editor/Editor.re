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
  isAnimated: bool,
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
  preview: bool,
  scrollbarVerticalWidth: int,
  scrollbarHorizontalWidth: int,
  wrapPadding: option(float),
  // Number of lines to preserve before or after the cursor, when scrolling.
  // Like the `scrolloff` vim setting or the `editor.cursorSurroundingLines` VSCode setting.
  verticalScrollMargin: int,
  // Mouse state
  isMouseDown: bool,
  hasMouseEntered: bool,
  mouseDownBytePosition: option(BytePosition.t),
  // The last mouse position, in screen coordinates
  lastMouseScreenPosition: option(PixelPosition.t),
  lastMouseMoveTime: [@opaque] option(Revery.Time.t),
  lastMouseUpTime: [@opaque] option(Revery.Time.t),
  // Animation
  isAnimationOverride: option(bool),
  animationNonce: int,
};

let verticalScrollbarThickness = ({scrollbarVerticalWidth, _}) => scrollbarVerticalWidth;
let horizontalScrollbarThickness = ({scrollbarHorizontalWidth, _}) => scrollbarHorizontalWidth;

let key = ({key, _}) => key;
// TODO: Handle multiple ranges
let selections = ({mode, _}) =>
  switch (mode) {
  | Visual(range) => [Oni_Core.VisualRange.ofVim(range)]

  | Select({ranges}) => ranges |> List.map(Oni_Core.VisualRange.ofVim)
  | _ => []
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

let overrideAnimation = (~animated, editor) => {
  ...editor,
  isAnimationOverride: animated,
};

let isAnimatingScroll = ({scrollX, scrollY, _}) => {
  Spring.isActive(scrollX) || Spring.isActive(scrollY);
};

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
  let wrapping = wrapState |> WrapState.wrapping;
  if (line < 0) {
    ({x: 0., y: 0.}: PixelPosition.t, 0.);
  } else if (line >= lineCount) {
    let pixelY =
      // Get total offset for view lines
      float(Wrapping.numberOfLines(wrapping))
      *. lineHeightInPixels(editor)
      // Add in codelens / inline elements
      +. InlineElements.getReservedSpace(
           position.line,
           editor.inlineElements,
         )
      -. scrollY
      +. 0.5;

    ({x: 0., y: pixelY}: PixelPosition.t, 0.);
  } else {
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
  animationNonce: editor.animationNonce + 1,
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
    scrollbarVerticalWidth,
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
      ~verticalScrollBarWidth=scrollbarVerticalWidth,
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
    EditorConfiguration.wordWrap.get(config) == `On
      ? WrapMode.Viewport : WrapMode.NoWrap;

  let scrolloff = EditorConfiguration.scrolloff.get(config);

  let isAnimated =
    Feature_Configuration.GlobalConfiguration.animation.get(config);
  let isScrollAnimated =
    isAnimated && EditorConfiguration.smoothScroll.get(config);

  let yankHighlightDuration =
    EditorConfiguration.yankHighlightDuration.get(config);

  let scrollbarVerticalWidth =
    EditorConfiguration.verticalScrollbarSize.get(config)
    |> IntEx.clamp(~hi=100, ~lo=1);

  let scrollbarHorizontalWidth =
    EditorConfiguration.horizontalScrollbarSize.get(config)
    |> IntEx.clamp(~hi=100, ~lo=1);

  // If codelens is turned off, remove all codelens keys

  let inlineElements =
    if (!
          Feature_Configuration.GlobalConfiguration.Editor.codeLensEnabled.get(
            config,
          )) {
      editor.inlineElements |> InlineElements.clearMatching(~key="codelens");
    } else {
      editor.inlineElements;
    };

  {
    ...editor,
    inlineElements,
    isAnimated,
    isScrollAnimated,
    scrollbarVerticalWidth,
    scrollbarHorizontalWidth,
    yankHighlightDuration,
  }
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

let create = (~config, ~buffer, ~preview: bool, ()) => {
  let id = GlobalState.generateId();
  let key = Brisk_reconciler.Key.create();

  let wrapMode =
    EditorConfiguration.wordWrap.get(config) == `On
      ? WrapMode.Viewport : WrapMode.NoWrap;

  let wrapState = WrapState.make(~pixelWidth=1000., ~wrapMode, ~buffer);

  let isAnimated =
    Feature_Configuration.GlobalConfiguration.animation.get(config);
  let isScrollAnimated =
    isAnimated && EditorConfiguration.smoothScroll.get(config);

  let yankHighlightDuration =
    EditorConfiguration.yankHighlightDuration.get(config);

  {
    editorId: id,
    key,
    lineHeight: LineHeight.default,
    lineNumbers: `On,
    isMinimapEnabled: true,
    isAnimated,
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
    preview,
    wrapPadding: None,
    verticalScrollMargin: 1,
    inlineElements: InlineElements.initial,
    isMouseDown: false,
    hasMouseEntered: false,
    lastMouseMoveTime: None,
    mouseDownBytePosition: None,
    lastMouseScreenPosition: None,
    lastMouseUpTime: None,

    // Animation
    isAnimationOverride: None,
    animationNonce: 0,

    scrollbarHorizontalWidth: 8,
    scrollbarVerticalWidth: 15,
  }
  |> configure(~config);
};

let cursors = ({mode, _}) => Vim.Mode.cursors(mode);

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
            accumulatedPixels
            +. additionalRegion
            +. Component_Animation.get(hd.height)
            +. lineHeight,
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

let singleLineSelectedText = editor => {
  let maybeSelection = editor |> selections |> (l => List.nth_opt(l, 0));

  let maybeByteRange =
    maybeSelection
    |> OptionEx.filter((selection: VisualRange.t) =>
         ByteRange.isSingleLine(selection.range)
       )
    |> OptionEx.flatMap((selection: VisualRange.t) => {
         switch (selection.mode) {
         | Vim.Types.Line
         | Vim.Types.Character => Some(selection.range)

         // TODO: Implement block range
         | Vim.Types.Block
         | Vim.Types.None => None
         }
       });

  maybeByteRange
  |> Option.map(ByteRange.normalize)
  |> OptionEx.flatMap(byteRange =>
       byteRangeToCharacterRange(byteRange, editor)
     )
  |> OptionEx.flatMap((characterRange: CharacterRange.t) => {
       let lineNumber = characterRange.start.line;
       let lineIdx = lineNumber |> EditorCoreTypes.LineNumber.toZeroBased;

       let buffer = editor.buffer;
       if (EditorBuffer.hasLine(lineNumber, buffer)) {
         let startIdx = characterRange.start.character |> CharacterIndex.toInt;
         let stopIdx = characterRange.stop.character |> CharacterIndex.toInt;

         let lineStr = EditorBuffer.line(lineIdx, buffer) |> BufferLine.raw;

         let len = ZedBundled.length(lineStr);
         let maxLength = len - startIdx;
         let desiredLength = stopIdx - startIdx + 1;
         let clampedLength = min(desiredLength, maxLength);
         let str = ZedBundled.sub(lineStr, startIdx, clampedLength);
         Some(str);
       } else {
         None;
       };
     });
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

let setCursors = (cursors, editor) => {
  {...editor, mode: Insert({cursors: cursors})};
};

let setSelections = (selections: list(ByteRange.t), editor) => {
  let mapRange = (r: ByteRange.t) => {
    open ByteRange;
    open Vim.VisualRange;
    let {start, stop}: ByteRange.t = r;
    {anchor: start, cursor: stop, visualType: Vim.Types.Character};
  };
  // Try to preserve the existing cursor, if we can
  let primaryCursor = getPrimaryCursorByte(editor);
  let primaryRanges =
    selections
    |> List.filter(range => ByteRange.contains(primaryCursor, range))
    |> List.map(mapRange);

  let secondaryRanges =
    selections
    |> List.filter(range => !ByteRange.contains(primaryCursor, range))
    |> List.map(mapRange);

  // Make sure primary ranges (ranges on the existing cursor) are first
  let ranges = primaryRanges @ secondaryRanges;

  {...editor, mode: Select({ranges: ranges})};
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

  let isAnimated = Spring.isActive(editor.scrollY);
  let scrollY =
    Spring.set(~instant=!isAnimated, ~position=scrollYValue, editor.scrollY);
  {
    ...editor',
    animationNonce:
      isAnimated ? editor.animationNonce + 1 : editor.animationNonce,
    scrollY,
  };
};

let makeInlineElement = (~key, ~uniqueId, ~lineNumber, ~view) => {
  hidden: false,
  reconcilerKey: Brisk_reconciler.Key.create(),
  key,
  uniqueId,
  lineNumber,
  view,
};

let linesWithInlineElements = ({inlineElements, _}) => {
  InlineElements.lines(inlineElements)
  |> List.map(EditorCoreTypes.LineNumber.ofZeroBased);
};

let getInlineElements = (~line, {inlineElements, _}) => {
  InlineElements.allElementsForLine(~line, inlineElements);
};

let setInlineElementSize =
    (~allowAnimation=true, ~key, ~line, ~uniqueId, ~height, editor) => {
  let topBuffer = getTopVisibleBufferLine(editor);
  let bottomBuffer = getBottomVisibleBufferLine(editor);
  editor
  |> withSteadyCursor(e =>
       {
         ...e,
         inlineElements:
           InlineElements.setSize(
             ~animated=
               allowAnimation
               && editor.isAnimated
               && line >= topBuffer
               && line <= bottomBuffer,
             ~key,
             ~line,
             ~uniqueId,
             ~height=float(height),
             e.inlineElements,
           ),
       }
     );
};

let replaceInlineElements = (~key, ~startLine, ~stopLine, ~elements, editor) => {
  let elements': list(InlineElements.element) =
    elements
    |> List.map((inlineElement: inlineElement) =>
         InlineElements.{
           key: inlineElement.key,
           uniqueId: inlineElement.uniqueId,
           line: inlineElement.lineNumber,
           height: Component_Animation.make(Animation.expand(0., 0.)),
           view: inlineElement.view,
           opacity: Component_Animation.make(Animation.fadeIn),
         }
       );
  editor
  |> withSteadyCursor(e =>
       {
         ...e,
         inlineElements:
           InlineElements.replace(
             ~startLine=Some(startLine),
             ~stopLine=Some(stopLine),
             ~key,
             ~elements=elements',
             e.inlineElements,
           ),
       }
     );
};

let setCodeLens = (~startLine, ~stopLine, ~handle, ~lenses, editor) => {
  let inlineElements =
    lenses
    |> List.map(lens => {
         let lineNumber =
           Feature_LanguageSupport.CodeLens.lineNumber(lens)
           |> EditorCoreTypes.LineNumber.ofZeroBased;
         let uniqueId = Feature_LanguageSupport.CodeLens.text(lens);
         let leftMargin =
           getLeadingWhitespacePixels(lineNumber, editor) |> int_of_float;
         let view =
           Feature_LanguageSupport.CodeLens.View.make(
             ~leftMargin,
             ~codeLens=lens,
           );
         makeInlineElement(
           ~key="codelens:" ++ string_of_int(handle),
           ~uniqueId,
           ~lineNumber,
           ~view,
         );
       });
  replaceInlineElements(
    ~startLine,
    ~stopLine,
    ~key="codelens:" ++ string_of_int(handle),
    ~elements=inlineElements,
    editor,
  );
};

let selectionOrCursorRange = editor => {
  switch (selections(editor)) {
  | [] =>
    let pos = getPrimaryCursorByte(editor);
    ByteRange.{
      start: BytePosition.{line: pos.line, byte: ByteIndex.zero},
      stop:
        BytePosition.{
          line: EditorCoreTypes.LineNumber.(pos.line + 1),
          byte: ByteIndex.zero,
        },
    };
  | [selection, ..._tail] => selection.range
  };
};

let getId = model => model.editorId;

let getPreview = model => model.preview;
let setPreview = (~preview, editor) => {...editor, preview};

let getVisibleView = editor => {
  let {pixelHeight, _} = editor;
  int_of_float(float_of_int(pixelHeight) /. lineHeightInPixels(editor));
};

let getTotalHeightInPixels = editor => {
  let totalViewLines = editor |> totalViewLines;
  int_of_float(float_of_int(totalViewLines) *. lineHeightInPixels(editor));
};

let getTotalWidthInPixels = ({wrapState, _} as editor) => {
  let contentPixelWidth = getContentPixelWidth(editor);
  let wrapping = wrapState |> WrapState.wrapping;
  max(Wrapping.maxLineLengthInPixels(wrapping), contentPixelWidth);
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
  let availableWidthF = float_of_int(availableWidth);
  let contentPixelWidth = getContentPixelWidth(editor);
  let totalViewWidthInPixels = getTotalWidthInPixels(editor);

  totalViewWidthInPixels <= contentPixelWidth
    ? {visible: false, thumbSize: 0, thumbOffset: 0}
    : {
      let thumbPercentage = availableWidthF /. totalViewWidthInPixels;
      let thumbSize = int_of_float(thumbPercentage *. availableWidthF);

      let topF = Spring.get(editor.scrollX) /. totalViewWidthInPixels;
      let thumbOffset = int_of_float(topF *. availableWidthF);

      // Clamp thumbsize to not exceed scrollable area
      let thumbSize' =
        if (thumbSize + thumbOffset > availableWidth) {
          let delta = thumbSize + thumbOffset - availableWidth;
          thumbSize - delta;
        } else {
          thumbSize;
        };

      {thumbSize: thumbSize', thumbOffset, visible: true};
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

let isScrollAnimated = ({isScrollAnimated, isAnimationOverride, _}) => {
  switch (isAnimationOverride) {
  | Some(v) => v
  | None => isScrollAnimated
  };
};

let synchronizeMinimapScroll = (~animated, editor) => {
  // Set up minimap scroll
  let newScrollY =
    animated ? Spring.get(editor.scrollY) : Spring.getTarget(editor.scrollY);
  let {pixelHeight, _} = editor;
  let viewLines = editor |> totalViewLines;
  let availableScroll =
    max(float_of_int(viewLines - 1), 0.)
    *. lineHeightInPixels(editor)
    +. InlineElements.getAllReservedSpace(editor.inlineElements);
  let scrollPercentage =
    newScrollY /. (availableScroll -. float_of_int(pixelHeight));
  let minimapLineSize =
    Constants.minimapCharacterWidth + Constants.minimapCharacterHeight;
  let linesInMinimap = pixelHeight / minimapLineSize;
  let availableMinimapScroll =
    max(viewLines - linesInMinimap, 0) * minimapLineSize;
  let minimapScrollY =
    scrollPercentage *. float_of_int(availableMinimapScroll);

  {...editor, minimapScrollY};
};

let exposePrimaryCursor = (~disableAnimation=false, editor) =>
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

      let animated = editor |> isScrollAnimated;
      {
        ...editor,
        scrollX:
          Spring.set(
            ~instant=!animated || disableAnimation,
            ~position=adjustedScrollX,
            editor.scrollX,
          ),
        scrollY:
          Spring.set(
            ~instant=!animated || disableAnimation,
            ~position=adjustedScrollY,
            editor.scrollY,
          ),
        animationNonce:
          animated ? editor.animationNonce + 1 : editor.animationNonce,
      }
      |> synchronizeMinimapScroll(~animated=animated && !disableAnimation);

    | _ => editor
    };
  };

// [isCursorFullyVisible] returns [true] if the cursor is
// visible and within [scrolloff] bounds, [false] otherwise.
let isCursorFullyVisible = editor => {
  // HACK: We just run 'exposePrimaryCursor', and see if it changes anything.
  let temporaryEditor = exposePrimaryCursor(editor);
  let preScrollX = Spring.getTarget(editor.scrollX);
  let preScrollY = Spring.getTarget(editor.scrollY);
  let postScrollX = Spring.getTarget(temporaryEditor.scrollX);
  let postScrollY = Spring.getTarget(temporaryEditor.scrollY);

  preScrollX == postScrollX && preScrollY == postScrollY;
};

let mode = ({mode, _}) => mode;

let setMode = (mode, editor) => {
  open EditorCoreTypes;
  let previousCursor = getPrimaryCursor(editor);
  let editor' = {...editor, mode};
  let newCursor = getPrimaryCursor(editor');

  if (CharacterPosition.equals(previousCursor, newCursor)) {
    editor';
  } else {
    // Check if we should animate the cursor.
    // The animation should be disabled for 'small jumps', ie:
    // - Single line movement
    // - Single character movement

    let lineDelta =
      CharacterPosition.(
        abs(
          LineNumber.toZeroBased(previousCursor.line)
          - LineNumber.toZeroBased(newCursor.line),
        )
      );
    let isSmallJumpLineWise = lineDelta <= 1;

    let isSmallJumpCharacterWise =
      previousCursor.line == newCursor.line
      && abs(
           CharacterIndex.toInt(previousCursor.character)
           - CharacterIndex.toInt(newCursor.character),
         )
      <= 1;
    let isSmallJump = isSmallJumpLineWise || isSmallJumpCharacterWise;

    // If it was a 'small jump', don't animate
    let disableAnimation = isSmallJump;
    editor' |> exposePrimaryCursor(~disableAnimation);
  };
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
      )
      |> Option.value(~default=character);
    let stopIndex =
      BufferLine.traverse(
        ~f,
        ~direction=`Forwards,
        ~index=character,
        bufferLine,
      )
      |> Option.value(~default=character);
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
  let originalScrollY = Spring.getTarget(editor.scrollY);
  let animated = editor |> isScrollAnimated && animated;
  let viewLines = editor |> totalViewLines;
  let newScrollY = max(0., newScrollY);
  let availableScroll =
    max(float_of_int(viewLines - 1), 0.)
    *. lineHeightInPixels(editor)
    +. InlineElements.getAllReservedSpace(editor.inlineElements);
  let newScrollY = min(newScrollY, availableScroll);

  // For small  jumps - ie, a single line, just teleport.
  let isSmallJump =
    !Spring.isActive(editor.scrollY)
    && Float.abs(newScrollY -. originalScrollY) < lineHeightInPixels(editor)
    *. 1.1;

  let instant = !animated || isSmallJump;
  {
    ...editor,
    animationNonce:
      instant ? editor.animationNonce : editor.animationNonce + 1,
    scrollY: Spring.set(~instant, ~position=newScrollY, editor.scrollY),
  }
  |> synchronizeMinimapScroll(~animated=!instant);
};

let scrollToLine = (~line, view) => {
  let pixelY = float_of_int(line) *. lineHeightInPixels(view);
  scrollToPixelY(~animated=true, ~pixelY, view);
};

let scrollToPixelX = (~animated, ~pixelX as newScrollX, editor) => {
  let animated = editor |> isScrollAnimated && animated;
  let newScrollX = max(0., newScrollX);

  let contentPixelWidth = getContentPixelWidth(editor);
  let totalWidthInPixels = getTotalWidthInPixels(editor);

  let availableScroll =
    if (totalWidthInPixels > contentPixelWidth) {
      getTotalWidthInPixels(editor)
      -. (contentPixelWidth -. getCharacterWidth(editor) *. 2.);
    } else {
      0.;
    };
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

let movePositionIntoView = (cursor: BytePosition.t, editor) => {
  let wrapping = editor.wrapState |> WrapState.wrapping;
  let currentViewLine =
    Wrapping.bufferBytePositionToViewLine(~bytePosition=cursor, wrapping);
  let topViewLine = getTopViewLine(editor);
  let bottomViewLine = getBottomViewLine(editor);
  let newViewLine =
    Utility.IntEx.clamp(
      ~lo=topViewLine + editor.verticalScrollMargin,
      ~hi=bottomViewLine - editor.verticalScrollMargin - 1,
      currentViewLine,
    );
  let {line: outLine, byteOffset, _}: Wrapping.bufferPosition =
    Wrapping.viewLineToBufferPosition(~line=newViewLine, wrapping);
  let line = EditorCoreTypes.LineNumber.toZeroBased(outLine);
  let bufferLineCount = EditorBuffer.numberOfLines(editor.buffer);
  let line' =
    Utility.IntEx.clamp(~lo=0, ~hi=bufferLineCount - 1, line)
    |> EditorCoreTypes.LineNumber.ofZeroBased;
  BytePosition.{line: line', byte: byteOffset};
};

let movePositionByViewLines =
    (~deltaViewLines=0, position: BytePosition.t, editor) => {
  let wrapping = editor.wrapState |> WrapState.wrapping;
  let currentViewLine =
    Wrapping.bufferBytePositionToViewLine(~bytePosition=position, wrapping);
  let totalViewLines = Wrapping.numberOfLines(wrapping);
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

let mapCursor = (~f, editor) => {
  let mode =
    switch (editor.mode) {
    // When scrolling in operator pending, cancel the pending operator
    | Operator({cursor, _}) => Vim.Mode.Normal({cursor: f(cursor)})
    // Don't do anything for command line mode
    | CommandLine({cursor, text, commandCursor, commandType}) =>
      CommandLine({text, commandCursor, commandType, cursor: f(cursor)})
    | Normal({cursor}) => Normal({cursor: f(cursor)})
    | Visual(curr) =>
      Visual(Vim.VisualRange.{...curr, cursor: f(curr.cursor)})
    | Select({ranges}) => Select({ranges: ranges})
    | Replace({cursor}) => Replace({cursor: f(cursor)})
    | Insert({cursors}) => Insert({cursors: List.map(f, cursors)})
    };

  {...editor, mode};
};

let scrollAndMoveCursor = (~deltaViewLines, editor) => {
  let oldCursor = getPrimaryCursorByte(editor);

  let adjustCursor = cursor =>
    movePositionByViewLines(~deltaViewLines, cursor, editor);

  // Move the cursor
  let editor' = mapCursor(~f=adjustCursor, editor);

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
  // Apply the scroll first - we might not need to move the cursor

  let scrollDelta = float(count) *. lineHeightInPixels(editor);
  let newScrollY = Spring.getTarget(editor.scrollY) +. scrollDelta;
  let editor' = scrollToPixelY(~animated=true, ~pixelY=newScrollY, editor);

  let didScroll =
    Spring.getTarget(editor'.scrollY) != Spring.getTarget(editor.scrollY);

  // Then, if needed, bump the cursor position - only if we actually scrolled, and the cursor isn't fully in view.
  if (!isCursorFullyVisible(editor') && didScroll) {
    let adjustCursor = cursor => movePositionIntoView(cursor, editor');
    mapCursor(~f=adjustCursor, editor');
  } else {
    editor';
  };
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
    editor'
    |> overrideAnimation(~animated=Some(false))
    |> scrollCursorTop
    |> overrideAnimation(~animated=None);
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
  let totalWidth = getTotalWidthInPixels(editor);
  let x = totalWidth *. pixelX /. float_of_int(pixelWidth);

  let totalHeight = getTotalHeightInPixels(editor) |> float_of_int;
  let y = totalHeight *. pixelY /. float_of_int(pixelHeight);

  (x, y);
};

let getBufferId = ({buffer, _}) => EditorBuffer.id(buffer);

let updateBuffer = (~update, ~markerUpdate, ~buffer, editor) => {
  {
    ...editor,
    buffer,
    wrapState: WrapState.update(~update, ~buffer, editor.wrapState),
    inlineElements:
      InlineElements.moveMarkers(markerUpdate, editor.inlineElements),
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

let mouseDown = (~altKey, ~time, ~pixelX, ~pixelY, editor) => {
  ignore(altKey);
  ignore(time);
  let bytePosition: BytePosition.t =
    Slow.pixelPositionToBytePosition(
      ~allowPast=true,
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
  {
    ...editor,
    mode,
    isMouseDown: true,
    mouseDownBytePosition: Some(bytePosition),
  };
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

       let lineIdx =
         EditorCoreTypes.LineNumber.toZeroBased(bytePosition.line);
       if (lineIdx < EditorBuffer.numberOfLines(editor.buffer)) {
         let bufferLine = EditorBuffer.line(lineIdx, editor.buffer);
         if (BufferLine.lengthInBytes(bufferLine)
             > ByteIndex.toInt(bytePosition.byte)) {
           byteToCharacter(bytePosition, editor);
         } else {
           None;
         };
       } else {
         None;
       };
     });
};

let mouseUp = (~altKey, ~time, ~pixelX, ~pixelY, editor) => {
  ignore(altKey);
  ignore(pixelX);
  ignore(pixelY);

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
             Vim.Mode.Select({ranges: [visualRange]});
           } else {
             Vim.Mode.Visual(visualRange);
           };
         })
      |> Option.value(~default=editor.mode);

    {
      ...editor,
      isMouseDown: false,
      mode,
      lastMouseUpTime: None,
      mouseDownBytePosition: None,
    };
  } else {
    {
      ...editor,
      isMouseDown: false,
      lastMouseUpTime: Some(time),
      mouseDownBytePosition: None,
    };
  };
};

let mouseMove = (~time, ~pixelX, ~pixelY, editor) => {
  let mode' =
    editor.mouseDownBytePosition
    |> Option.map(pos => {
         let isInsertMode = Vim.Mode.isInsert(editor.mode);
         let isSelectMode = Vim.Mode.isSelect(editor.mode);

         let newPosition =
           Slow.pixelPositionToBytePosition(
             // #2463: When we're insert mode, clicking past the end of the line
             // should move the cursor past the last byte
             ~allowPast=isInsertMode,
             ~pixelX,
             ~pixelY,
             editor,
           );

         let visualRange =
           Vim.VisualRange.{
             cursor: newPosition,
             anchor: pos,
             visualType: Vim.Types.Character,
           };

         if (isInsertMode || isSelectMode) {
           if (newPosition == pos) {
             Vim.Mode.Insert({cursors: [newPosition]});
           } else {
             Vim.Mode.Select({ranges: [visualRange]});
           };
         } else if (newPosition == pos) {
           Vim.Mode.Normal({cursor: newPosition});
         } else {
           Vim.Mode.Visual(visualRange);
         };
       })
    |> Option.value(~default=editor.mode);

  {
    ...editor,
    mode: mode',
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

let autoScroll = (~deltaPixelX: float, ~deltaPixelY: float, editor) => {
  // Scroll editor to new position
  let editor' =
    editor
    |> scrollDeltaPixelXY(
         ~animated=true,
         ~pixelX=deltaPixelX,
         ~pixelY=deltaPixelY,
       );

  // Simulate a mouse move if the mouse is pressed, to readjust selection

  OptionEx.map2(
    (time, position: PixelPosition.t) => {
      editor' |> mouseMove(~time, ~pixelX=position.x, ~pixelY=position.y)
    },
    editor'.lastMouseMoveTime,
    editor'.lastMouseScreenPosition,
  )
  |> Option.value(~default=editor');
};

[@deriving show]
type msg =
  | Animation([@opaque] Component_Animation.msg)
  | AutoScroll({
      deltaPixelY: float,
      deltaPixelX: float,
    });

let update = (msg, editor) => {
  switch (msg) {
  | AutoScroll({deltaPixelY, deltaPixelX}) =>
    autoScroll(~deltaPixelX, ~deltaPixelY, editor)

  | Animation(msg) =>
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

    let editor' =
      {
        ...editor,
        scrollX: Spring.update(msg, editor.scrollX),
        scrollY: Spring.update(msg, editor.scrollY),
        yankHighlight: yankHighlight',
        animationNonce: editor.animationNonce + 1,
      }
      |> synchronizeMinimapScroll(~animated=true);

    editor'
    |> withSteadyCursor(e =>
         {
           ...e,
           inlineElements: InlineElements.animate(msg, editor.inlineElements),
         }
       );
  };
};

let isMousePressedNearTop = ({isMouseDown, lastMouseScreenPosition, _}) => {
  isMouseDown
  && lastMouseScreenPosition
  |> Option.map(({y, _}: PixelPosition.t) => {
       int_of_float(y) < Constants.mouseAutoScrollBorder
     })
  |> Option.value(~default=false);
};

let isMousePressedNearBottom =
    ({isMouseDown, lastMouseScreenPosition, pixelHeight, _}) => {
  isMouseDown
  && lastMouseScreenPosition
  |> Option.map(({y, _}: PixelPosition.t) => {
       int_of_float(y) > pixelHeight - Constants.mouseAutoScrollBorder
     })
  |> Option.value(~default=false);
};

let isMousePressedNearLeftEdge = ({isMouseDown, lastMouseScreenPosition, _}) => {
  isMouseDown
  && lastMouseScreenPosition
  |> Option.map(({x, _}: PixelPosition.t) => {
       int_of_float(x) < Constants.mouseAutoScrollBorder
     })
  |> Option.value(~default=false);
};

let isMousePressedNearRightEdge =
    ({isMouseDown, lastMouseScreenPosition, _} as editor) => {
  let {bufferWidthInPixels, _}: EditorLayout.t = getLayout(editor);
  isMouseDown
  && lastMouseScreenPosition
  |> Option.map(({x, _}: PixelPosition.t) => {
       x > bufferWidthInPixels -. float(Constants.mouseAutoScrollBorder)
     })
  |> Option.value(~default=false);
};

let sub = editor => {
  let isYankAnimating =
    yankHighlight(editor)
    |> Option.map(({opacity, _}) => {
         opacity |> Component_Animation.isActive
       })
    |> Option.value(~default=false);

  let isInlineElementAnimating =
    InlineElements.isAnimating(editor.inlineElements);

  let isScrollAnimating =
    Spring.isActive(editor.scrollY) || Spring.isActive(editor.scrollX);

  let maybeAutoScrollUp =
    if (isMousePressedNearTop(editor)) {
      Some(
        Service_Time.Sub.interval(
          ~uniqueId="AutoScrollUp" ++ string_of_int(editor.editorId),
          ~every=Constants.mouseAutoScrollInterval,
          ~msg=(~current as _) => {
          AutoScroll({
            deltaPixelY: (-1.) *. Constants.mouseAutoScrollSpeed,
            deltaPixelX: 0.,
          })
        }),
      );
    } else {
      None;
    };

  let maybeAutoScrollDown =
    if (isMousePressedNearBottom(editor)) {
      Some(
        Service_Time.Sub.interval(
          ~uniqueId="AutoScrollDown" ++ string_of_int(editor.editorId),
          ~every=Constants.mouseAutoScrollInterval,
          ~msg=(~current as _) => {
          AutoScroll({
            deltaPixelY: Constants.mouseAutoScrollSpeed,
            deltaPixelX: 0.,
          })
        }),
      );
    } else {
      None;
    };

  let maybeAutoScrollLeft =
    if (isMousePressedNearLeftEdge(editor)
        && Component_Animation.Spring.get(editor.scrollX) > 0.) {
      Some(
        Service_Time.Sub.interval(
          ~uniqueId="AutoScrollLeft" ++ string_of_int(editor.editorId),
          ~every=Constants.mouseAutoScrollInterval,
          ~msg=(~current as _) => {
          AutoScroll({
            deltaPixelY: 0.,
            deltaPixelX: (-1.) *. Constants.mouseAutoScrollSpeed,
          })
        }),
      );
    } else {
      None;
    };

  let maybeAutoScrollRight =
    if (isMousePressedNearRightEdge(editor)) {
      Some(
        Service_Time.Sub.interval(
          ~uniqueId="AutoScrollRight" ++ string_of_int(editor.editorId),
          ~every=Constants.mouseAutoScrollInterval,
          ~msg=(~current as _) => {
          AutoScroll({
            deltaPixelY: 0.,
            deltaPixelX: Constants.mouseAutoScrollSpeed,
          })
        }),
      );
    } else {
      None;
    };
  let autoScrollSubs =
    [
      maybeAutoScrollUp,
      maybeAutoScrollDown,
      maybeAutoScrollLeft,
      maybeAutoScrollRight,
    ]
    |> List.filter_map(Fun.id);

  let animationSub =
    if (isYankAnimating || isInlineElementAnimating || isScrollAnimating) {
      Component_Animation.subAny(
        ~uniqueId=
          "editor."
          ++ string_of_int(editor.editorId)
          ++ "."
          ++ string_of_int(editor.animationNonce),
      )
      |> Isolinear.Sub.map(msg => Animation(msg));
    } else {
      Isolinear.Sub.none;
    };

  [animationSub, ...autoScrollSubs] |> Isolinear.Sub.batch;
};
