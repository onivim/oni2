open EditorCoreTypes;
open Oni_Core;

[@deriving show]
type t;

type scrollbarMetrics = {
  visible: bool,
  thumbSize: int,
  thumbOffset: int,
};

type viewLine = {
  contents: BufferLine.t,
  byteOffset: int,
  characterOffset: int,
};

type yankHighlight = {
  key: Brisk_reconciler.Key.t,
  pixelRanges: list(PixelRange.t),
};

module WrapMode: {
  [@deriving show]
  type t =
    | NoWrap
    | Viewport
    | WrapColumn(int)
    | Bounded(int);
};

let create:
  (
    ~wrapMode: WrapMode.t=?,
    ~config: Config.resolver,
    ~buffer: EditorBuffer.t,
    unit
  ) =>
  t;
let copy: t => t;

let key: t => Brisk_reconciler.Key.t;
let getId: t => int;
let getBufferId: t => int;
let getTopVisibleLine: t => int;
let getBottomVisibleLine: t => int;
let getLeftVisibleColumn: t => int;
let getLayout:
  (~showLineNumbers: bool, ~maxMinimapCharacters: int, t) => EditorLayout.t;
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
let setCursors: (~cursors: list(BytePosition.t), t) => t;

let getTokenAt:
  (~languageConfiguration: LanguageConfiguration.t, CharacterPosition.t, t) =>
  option(CharacterRange.t);

let yankHighlight: t => option(yankHighlight);
let setYankHighlight: (~yankHighlight: yankHighlight, t) => t;

let isMinimapEnabled: t => bool;
let setMinimapEnabled: (~enabled: bool, t) => t;

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

let viewLine: (t, int) => viewLine;

let scrollX: t => float;
let scrollY: t => float;
let minimapScrollY: t => float;

let lineHeightInPixels: t => float;
let linePaddingInPixels: t => float;
let setLineHeight: (~lineHeight: LineHeight.t, t) => t;
let characterWidthInPixels: t => float;

let selection: t => option(VisualRange.t);
let setSelection: (~selection: VisualRange.t, t) => t;
let clearSelection: t => t;

let selectionOrCursorRange: t => ByteRange.t;

let totalViewLines: t => int;

let isScrollAnimated: t => bool;
let scrollToColumn: (~column: int, t) => t;
let scrollToPixelX: (~pixelX: float, t) => t;
let scrollDeltaPixelX: (~pixelX: float, t) => t;

let scrollToLine: (~line: int, t) => t;
let scrollToPixelY: (~pixelY: float, t) => t;
let scrollDeltaPixelY: (~pixelY: float, t) => t;

let scrollToPixelXY: (~pixelX: float, ~pixelY: float, t) => t;
let scrollDeltaPixelXY: (~pixelX: float, ~pixelY: float, t) => t;

let getCharacterWidth: t => float;

// BYTE-CHARACTER CONVERSION
let byteToCharacter: (BytePosition.t, t) => option(CharacterPosition.t);
let characterToByte: (CharacterPosition.t, t) => option(BytePosition.t);

let byteRangeToCharacterRange: (ByteRange.t, t) => option(CharacterRange.t);

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

module Slow: {
  let pixelPositionToBytePosition:
    // Allow the return value to exceed the byte position of the line
    // This makes sense for cases like insert mode, where the cursor could be 'after'
    // the end of the line.
    (
      ~allowPast: bool=?,
      ~buffer: Buffer.t,
      ~pixelX: float,
      ~pixelY: float,
      t
    ) =>
    BytePosition.t;
};
