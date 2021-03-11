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
  opacity: Component_Animation.t(float),
};

module WrapMode: {
  [@deriving show]
  type t =
    | NoWrap
    | Viewport;
};

let create:
  (~config: Config.resolver, ~buffer: EditorBuffer.t, ~preview: bool, unit) =>
  t;
let copy: t => t;

type inlineElement;

let makeInlineElement:
  (
    ~key: string,
    ~uniqueId: string,
    ~lineNumber: EditorCoreTypes.LineNumber.t,
    ~view: (~theme: Oni_Core.ColorTheme.Colors.t, ~uiFont: UiFont.t, unit) =>
           Revery.UI.element
  ) =>
  inlineElement;

let setCodeLens:
  (
    ~startLine: EditorCoreTypes.LineNumber.t,
    ~stopLine: EditorCoreTypes.LineNumber.t,
    ~handle: int,
    ~lenses: list(Feature_LanguageSupport.CodeLens.t),
    t
  ) =>
  t;

let setInlineElementSize:
  (
    ~allowAnimation: bool=?,
    ~key: string,
    ~line: EditorCoreTypes.LineNumber.t,
    ~uniqueId: string,
    ~height: int,
    t
  ) =>
  t;

let getInlineElements:
  (~line: EditorCoreTypes.LineNumber.t, t) => list(InlineElements.element);

let linesWithInlineElements: t => list(EditorCoreTypes.LineNumber.t);

let key: t => Brisk_reconciler.Key.t;
let getId: t => int;
let getPreview: t => bool;
let setPreview: (~preview: bool, t) => t;
let getBufferId: t => int;
let getTopVisibleBufferLine: t => EditorCoreTypes.LineNumber.t;
let getBottomVisibleBufferLine: t => EditorCoreTypes.LineNumber.t;
let getTopViewLine: t => int;
let getBottomViewLine: t => int;
let getLeftVisibleColumn: t => int;
let getLayout: t => EditorLayout.t;
let getCharacterUnderCursor: t => option(Uchar.t);
let getCharacterBehindCursor: t => option(Uchar.t);
let getCharacterAtPosition:
  (~position: CharacterPosition.t, t) => option(Uchar.t);
let getPrimaryCursor: t => CharacterPosition.t;
let getPrimaryCursorByte: t => BytePosition.t;
let cursors: t => list(BytePosition.t);
let getVisibleView: t => int;
let getTotalHeightInPixels: t => int;
let getTotalWidthInPixels: t => float;
let getVerticalScrollbarMetrics: (t, int) => scrollbarMetrics;
let getHorizontalScrollbarMetrics: (t, int) => scrollbarMetrics;
let getCursors: t => list(BytePosition.t);
let setWrapMode: (~wrapMode: WrapMode.t, t) => t;

let setCursors: (list(BytePosition.t), t) => t;
let setSelections: (list(ByteRange.t), t) => t;

let horizontalScrollbarThickness: t => int;
let verticalScrollbarThickness: t => int;

// Get the horizontal width in pixels of the tab/space whitespace in front of a line.
let getLeadingWhitespacePixels: (EditorCoreTypes.LineNumber.t, t) => float;

let mode: t => Vim.Mode.t;
let setMode: (Vim.Mode.t, t) => t;

let getBufferLineCount: t => int;

let isAnimatingScroll: t => bool;

let getTokenAt:
  (~languageConfiguration: LanguageConfiguration.t, CharacterPosition.t, t) =>
  option(CharacterRange.t);

let overrideAnimation: (~animated: option(bool), t) => t;
let yankHighlight: t => option(yankHighlight);
let startYankHighlight: (list(PixelRange.t), t) => t;

let setWrapPadding: (~padding: float, t) => t;
let setVerticalScrollMargin: (~lines: int, t) => t;

let setMinimap: (~enabled: bool, ~maxColumn: int, t) => t;
let isMinimapEnabled: t => bool;

// Mouse interactions
let mouseDown:
  (~altKey: bool, ~time: Revery.Time.t, ~pixelX: float, ~pixelY: float, t) => t;
let mouseUp:
  (~altKey: bool, ~time: Revery.Time.t, ~pixelX: float, ~pixelY: float, t) => t;
let mouseMove: (~time: Revery.Time.t, ~pixelX: float, ~pixelY: float, t) => t;
let mouseEnter: t => t;
let mouseLeave: t => t;
let hasMouseEntered: t => bool;
let isMouseDown: t => bool;
let lastMouseMoveTime: t => option(Revery.Time.t);
let getCharacterUnderMouse: t => option(CharacterPosition.t);

// Scale factor between horizontal pixels on the editor surface vs minimap
let getMinimapWidthScaleFactor: t => float;

let setLineNumbers:
  (~lineNumbers: [ | `Off | `On | `Relative | `RelativeOnly], t) => t;
let lineNumbers: t => [ | `Off | `On | `Relative | `RelativeOnly];

// [exposePrimaryCursor(editor)] ensures the primary cursor is visible - adjusting the scroll if it isnot.
let exposePrimaryCursor: (~disableAnimation: bool=?, t) => t;

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

let selections: t => list(VisualRange.t);

let selectionOrCursorRange: t => ByteRange.t;

let totalViewLines: t => int;

let scrollToPixelX: (~animated: bool, ~pixelX: float, t) => t;
let scrollDeltaPixelX: (~animated: bool, ~pixelX: float, t) => t;

let scrollToLine: (~line: int, t) => t;
let scrollToPixelY: (~animated: bool, ~pixelY: float, t) => t;
let scrollDeltaPixelY: (~animated: bool, ~pixelY: float, t) => t;

let scrollToPixelXY: (~animated: bool, ~pixelX: float, ~pixelY: float, t) => t;
let scrollDeltaPixelXY:
  (~animated: bool, ~pixelX: float, ~pixelY: float, t) => t;

let scrollCenterCursorVertically: t => t;
let scrollCursorTop: t => t;
let scrollCursorBottom: t => t;
let scrollLines: (~count: int, t) => t;
let scrollHalfPage: (~count: int, t) => t;
let scrollPage: (~count: int, t) => t;

let getCharacterWidth: t => float;

let singleLineSelectedText: t => option(string);

// Given a start position and a delta screen lines,
// figure out a destination byte position.
let moveScreenLines:
  (~position: BytePosition.t, ~count: int, t) => BytePosition.t;

// BYTE-CHARACTER CONVERSION
let byteToCharacter: (BytePosition.t, t) => option(CharacterPosition.t);
let characterToByte: (CharacterPosition.t, t) => option(BytePosition.t);

let byteRangeToCharacterRange: (ByteRange.t, t) => option(CharacterRange.t);
let characterRangeToByteRange: (CharacterRange.t, t) => option(ByteRange.t);

// VIEW-SPACE CONVERSION

// [viewLineIsPrimary(viewLine, editor)] returns [true] if the viewline is
// the first view line for a buffer line (ie, its [byteOffset] is [0])
let viewLineIsPrimary: (int, t) => bool;
let viewLineToBufferLine: (int, t) => EditorCoreTypes.LineNumber.t;
let bufferBytePositionToViewLine: (BytePosition.t, t) => int;
let viewLineToPixelY: (int, t) => float;

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
  (
    ~update: Oni_Core.BufferUpdate.t,
    ~markerUpdate: Oni_Core.MarkerUpdate.t,
    ~buffer: EditorBuffer.t,
    t
  ) =>
  t;
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

[@deriving show]
type msg;

let update: (msg, t) => t;

let sub: t => Isolinear.Sub.t(msg);
