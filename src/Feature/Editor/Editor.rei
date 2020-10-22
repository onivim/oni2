open EditorCoreTypes;
open Oni_Core;

[@deriving show]
type t;

type scrollbarMetrics = {
  visible: bool,
  thumbSize: int,
  thumbOffset: int,
};

type yankHighlight = {
  key: Brisk_reconciler.Key.t,
  pixelRanges: list(PixelRange.t),
};

module WrapMode: {
  [@deriving show]
  type t =
    | NoWrap
    | Viewport;
};

let create: (~config: Config.resolver, ~buffer: EditorBuffer.t, unit) => t;
let copy: t => t;

let key: t => Brisk_reconciler.Key.t;
let getId: t => int;
let getBufferId: t => int;
let getTopVisibleBufferLine: t => EditorCoreTypes.LineNumber.t;
let getBottomVisibleBufferLine: t => EditorCoreTypes.LineNumber.t;
let getLeftVisibleColumn: t => int;
let getLayout: t => EditorLayout.t;
let getCharacterUnderCursor: t => option(Uchar.t);
let getCharacterBehindCursor: t => option(Uchar.t);
let getCharacterAtPosition:
  (~position: CharacterPosition.t, t) => option(Uchar.t);
let getPrimaryCursor: t => CharacterPosition.t;
let getPrimaryCursorByte: t => BytePosition.t;
let getVisibleView: t => int;
let getTotalHeightInPixels: t => int;
let getTotalWidthInPixels: t => int;
let getVerticalScrollbarMetrics: (t, int) => scrollbarMetrics;
let getHorizontalScrollbarMetrics: (t, int) => scrollbarMetrics;
let getCursors: t => list(BytePosition.t);
let setWrapMode: (~wrapMode: WrapMode.t, t) => t;

let mode: t => Vim.Mode.t;
let setMode: (Vim.Mode.t, t) => t;

let getBufferLineCount: t => int;

let getTokenAt:
  (~languageConfiguration: LanguageConfiguration.t, CharacterPosition.t, t) =>
  option(CharacterRange.t);

let yankHighlight: t => option(yankHighlight);
let setYankHighlight: (~yankHighlight: yankHighlight, t) => t;

let setWrapPadding: (~padding: float, t) => t;
let setVerticalScrollMargin: (~lines: int, t) => t;

let setMinimap: (~enabled: bool, ~maxColumn: int, t) => t;
let isMinimapEnabled: t => bool;

// Scale factor between horizontal pixels on the editor surface vs minimap
let getMinimapWidthScaleFactor: t => float;

let setLineNumbers:
  (~lineNumbers: [ | `Off | `On | `Relative | `RelativeOnly], t) => t;
let lineNumbers: t => [ | `Off | `On | `Relative | `RelativeOnly];

// [exposePrimaryCursor(editor)] ensures the primary cursor is visible - adjusting the scroll if it isnot.
let exposePrimaryCursor: t => t;

let getNearestMatchingPair:
  (
    ~characterPosition: CharacterPosition.t,
    ~pairs: list(LanguageConfiguration.BracketPair.t),
    t
  ) =>
  option((CharacterPosition.t, CharacterPosition.t));

let visiblePixelWidth: t => int;
let visiblePixelHeight: t => int;

let font: t => Service_Font.font;

let viewTokens:
  (~line: int, ~scrollX: float, ~colorizer: BufferLineColorizer.t, t) =>
  list(BufferViewTokenizer.t);

let scrollX: t => float;
let scrollY: t => float;
let minimapScrollY: t => float;

let lineHeightInPixels: t => float;
let linePaddingInPixels: t => float;
let setLineHeight: (~lineHeight: LineHeight.t, t) => t;
let characterWidthInPixels: t => float;

let selection: t => option(VisualRange.t);

let selectionOrCursorRange: t => ByteRange.t;

let totalViewLines: t => int;

let isScrollAnimated: t => bool;
let scrollToPixelX: (~pixelX: float, t) => t;
let scrollDeltaPixelX: (~pixelX: float, t) => t;

let scrollToLine: (~line: int, t) => t;
let scrollToPixelY: (~pixelY: float, t) => t;
let scrollDeltaPixelY: (~pixelY: float, t) => t;

let scrollToPixelXY: (~pixelX: float, ~pixelY: float, t) => t;
let scrollDeltaPixelXY: (~pixelX: float, ~pixelY: float, t) => t;

let scrollCenterCursorVertically: t => t;
let scrollCursorTop: t => t;
let scrollCursorBottom: t => t;
let scrollLines: (~count: int, t) => t;
let scrollHalfPage: (~count: int, t) => t;
let scrollPage: (~count: int, t) => t;

let getCharacterWidth: t => float;

// Given a start position and a delta screen lines,
// figure out a destination byte position.
let moveScreenLines:
  (~position: BytePosition.t, ~count: int, t) => BytePosition.t;

// BYTE-CHARACTER CONVERSION
let byteToCharacter: (BytePosition.t, t) => option(CharacterPosition.t);
let characterToByte: (CharacterPosition.t, t) => option(BytePosition.t);

let byteRangeToCharacterRange: (ByteRange.t, t) => option(CharacterRange.t);

// VIEW-SPACE CONVERSION

// [viewLineIsPrimary(viewLine, editor)] returns [true] if the viewline is
// the first view line for a buffer line (ie, its [byteOffset] is [0])
let viewLineIsPrimary: (int, t) => bool;
let viewLineToBufferLine: (int, t) => EditorCoreTypes.LineNumber.t;
let bufferBytePositionToViewLine: (BytePosition.t, t) => int;

// PIXEL-SPACE CONVERSION

// These methods convert a buffer (line, byte) or (line, utf8 character index)
// to a pixel position on-screen - accounting for word wrap, folding, scrolling, etc.

// They return both the pixel position, as well as the character width of the target character.
let bufferBytePositionToPixel:
  (~position: BytePosition.t, t) => (PixelPosition.t, float);
let bufferCharacterPositionToPixel:
  (~position: CharacterPosition.t, t) => (PixelPosition.t, float);

// PROJECTION

// Convert (or unconvent) a (line, column) to a different coordinate space

// [project] - given a zero-based [line] and [column], and a pixel space
// defined by [pixelWidth] and [pixelHeight], return the [(pixelX, pixelY)]
// corresponding to the top-left of the line and column.
let project:
  (
    ~line: EditorCoreTypes.LineNumber.t,
    ~column: int,
    ~pixelWidth: int,
    ~pixelHeight: int,
    t
  ) =>
  (float, float);

// [projectLine] - like [project], but ignoring the [column]/[width]
let projectLine:
  (~line: EditorCoreTypes.LineNumber.t, ~pixelHeight: int, t) => float;

// [unproject] - given a pixel space defined by [pixelWidth] and [pixelHeight],
// map the [pixelX] and [pixelY] of that space to a pixel position on the editor
// surface - [(surfacePixelX, surfacePixelY)]

let unprojectToPixel:
  (~pixelX: float, ~pixelY: float, ~pixelWidth: int, ~pixelHeight: int, t) =>
  (float, float);

let setSize: (~pixelWidth: int, ~pixelHeight: int, t) => t;

let updateBuffer:
  (~update: Oni_Core.BufferUpdate.t, ~buffer: EditorBuffer.t, t) => t;
let setBuffer: (~buffer: EditorBuffer.t, t) => t;

let configurationChanged:
  (~perFileTypeConfig: Oni_Core.Config.fileTypeResolver, t) => t;

module Slow: {
  let pixelPositionToBytePosition:
    // Allow the return value to exceed the byte position of the line
    // This makes sense for cases like insert mode, where the cursor could be 'after'
    // the end of the line.
    (~allowPast: bool=?, ~pixelX: float, ~pixelY: float, t) => BytePosition.t;
};
