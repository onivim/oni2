open EditorCoreTypes;
open Oni_Core;
module LineNumber = EditorCoreTypes.LineNumber;

type bufferPosition = {
  line: LineNumber.t,
  byteOffset: ByteIndex.t,
  characterOffset: CharacterIndex.t,
};

type t = {
  wrap: WordWrap.t,
  buffer: EditorBuffer.t,
  // Per-buffer-line array of wrap points
  wraps: array((array(WordWrap.lineWrap), float)),
  wrapsMutationCount: int,
  // Map of buffer line index -> view line index
  bufferLineToViewLineCache: IntMap.t(int),
  // Map of view line index -> buffer index
  viewLineToBufferCache: IntMap.t(int),
  totalViewLines: int,
  // The maximum length in pixels, of any line
  maxLengthInPixels: float,
};

module Internal = {
  let bufferToWraps = (~wrap, buffer) => {
    let bufferLineCount = EditorBuffer.numberOfLines(buffer);
    let wraps = Array.make(bufferLineCount, ([||], 0.));

    for (idx in 0 to bufferLineCount - 1) {
      let line = EditorBuffer.line(idx, buffer);
      wraps[idx] = wrap(line);
    };
    wraps;
  };

  let recalculateCaches =
      (~wraps: array((array(WordWrap.lineWrap), float))) => {
    let len = Array.length(wraps);

    let rec addViewLines = (map, bufferLine, currentViewLine, stopViewLine) =>
      if (currentViewLine == stopViewLine) {
        map;
      } else {
        let map' = IntMap.add(currentViewLine, bufferLine, map);
        addViewLines(map', bufferLine, currentViewLine + 1, stopViewLine);
      };

    let rec loop = (acc, idx) =>
      if (idx == len) {
        acc;
      } else {
        let (map, viewLineMap, count) = acc;
        let (lineWraps, _totalPixelSize) = wraps[idx];
        let wrapCount = lineWraps |> Array.length;
        let map' = IntMap.add(idx, count, map);
        let count' = count + wrapCount;

        let viewLineMap' = addViewLines(viewLineMap, idx, count, count');

        loop((map', viewLineMap', count'), idx + 1);
      };

    loop((IntMap.empty, IntMap.empty, 0), 0);
  };

  let bufferLineToViewLine =
      (bufferLine, {bufferLineToViewLineCache, totalViewLines, _}) => {
    IntMap.find_opt(bufferLine, bufferLineToViewLineCache)
    |> Option.value(~default=totalViewLines);
  };

  let viewLineToBufferLine = (viewLine, {wraps, viewLineToBufferCache, _}) => {
    let len = Array.length(wraps);
    IntMap.find_opt(viewLine, viewLineToBufferCache)
    |> Option.value(~default=len);
  };

  let recalculateMaxLineSize =
      (wraps: array((array(WordWrap.lineWrap), float))) => {
    let len = Array.length(wraps);
    let max = ref(0.);
    for (idx in 0 to len - 1) {
      let (_wraps, pixelSize) = wraps[idx];
      if (pixelSize > max^) {
        max := pixelSize;
      };
    };
    max^;
  };
};

let make = (~wrap: Oni_Core.WordWrap.t, ~buffer) => {
  let wraps = Internal.bufferToWraps(~wrap, buffer);
  let (bufferLineToViewLineCache, viewLineToBufferCache, totalViewLines) =
    Internal.recalculateCaches(~wraps);
  {
    wrap,
    wrapsMutationCount: 0,
    buffer,
    wraps,
    bufferLineToViewLineCache,
    viewLineToBufferCache,
    totalViewLines,
    maxLengthInPixels: Internal.recalculateMaxLineSize(wraps),
  };
};

let update =
    (
      ~update: Oni_Core.BufferUpdate.t,
      ~newBuffer,
      {wrap, wraps, _} as wrapping: t,
    ) => {
  let startLine = update.startLine |> LineNumber.toZeroBased;
  let endLine = update.endLine |> LineNumber.toZeroBased;
  // Special case - the number of lines haven't changed. We can streamline this.
  if (!update.isFull
      && endLine
      - startLine == Array.length(update.lines)
      && Array.length(wraps) >= endLine) {
    // Update lines in update
    let isRecalculationNeeded = ref(false);
    for (idx in startLine to endLine - 1) {
      // Check previous wrap count
      let (previousWraps, _pixelSize) = wraps[idx];
      let wrapCount = previousWraps |> Array.length;

      let line = EditorBuffer.line(idx, newBuffer);

      // HACK: Mutation

      let newWraps = wrap(line);
      let (newLineWraps, _pixelSize) = wrap(line);
      let newWrapCount = newLineWraps |> Array.length;
      wraps[idx] = newWraps;

      if (newWrapCount != wrapCount) {
        isRecalculationNeeded := true;
      };
    };

    let (bufferLineToViewLineCache, viewLineToBufferCache, totalViewLines) =
      if (isRecalculationNeeded^ == false) {
        (
          wrapping.bufferLineToViewLineCache,
          wrapping.viewLineToBufferCache,
          wrapping.totalViewLines,
        );
      } else {
        Internal.recalculateCaches(~wraps);
      };

    {
      ...wrapping,
      wrapsMutationCount: wrapping.wrapsMutationCount + 1,
      bufferLineToViewLineCache,
      viewLineToBufferCache,
      totalViewLines,
      maxLengthInPixels: Internal.recalculateMaxLineSize(wraps),
    };
  } else {
    let wraps = Internal.bufferToWraps(~wrap, newBuffer);
    let (bufferLineToViewLineCache, viewLineToBufferCache, totalViewLines) =
      Internal.recalculateCaches(~wraps);
    {
      wrap,
      wrapsMutationCount: 0,
      buffer: newBuffer,
      wraps,
      bufferLineToViewLineCache,
      viewLineToBufferCache,
      totalViewLines,
      maxLengthInPixels: Internal.recalculateMaxLineSize(wraps),
    };
  };
};

let bufferBytePositionToViewLine = (~bytePosition: BytePosition.t, wrap) => {
  let line = EditorCoreTypes.LineNumber.toZeroBased(bytePosition.line);
  let startViewLine = Internal.bufferLineToViewLine(line, wrap);
  let byteIndex = bytePosition.byte;

  if (line >= Array.length(wrap.wraps)) {
    startViewLine;
  } else {
    let (viewLines, _pixelSize) = wrap.wraps[line];

    let len = Array.length(viewLines);
    let rec loop = idx =>
      if (idx == len - 1) {
        idx;
      } else if (idx == len - 2) {
        let lastElement = viewLines[idx + 1];
        if (ByteIndex.(byteIndex < lastElement.byte)) {
          idx;
        } else {
          idx + 1;
        };
      } else {
        let current = viewLines[idx].byte;
        let next = viewLines[idx + 1].byte;
        if (byteIndex >= current && byteIndex < next) {
          idx;
        } else {
          loop(idx + 1);
        };
      };

    let offset = loop(0);
    startViewLine + offset;
  };
};

let viewLineToBufferPosition = (~line: int, wrapping) => {
  let line = max(0, line);
  let bufferLineIdx = Internal.viewLineToBufferLine(line, wrapping);
  let startViewLine = Internal.bufferLineToViewLine(bufferLineIdx, wrapping);

  let len = Array.length(wrapping.wraps);
  if (len == 0) {
    {
      line: LineNumber.zero,
      byteOffset: ByteIndex.zero,
      characterOffset: CharacterIndex.zero,
    };
  } else if (bufferLineIdx >= len) {
    {
      line:
        EditorBuffer.numberOfLines(wrapping.buffer) |> LineNumber.ofZeroBased,
      byteOffset: ByteIndex.zero,
      characterOffset: CharacterIndex.zero,
    };
  } else {
    let (wraps, _pixelSize) = wrapping.wraps[bufferLineIdx];

    let idx = line - startViewLine;
    let len = Array.length(wraps);
    let lineWrap =
      if (idx < len) {
        wraps[idx];
      } else {
        WordWrap.{byte: ByteIndex.zero, character: CharacterIndex.zero};
      };

    {
      line: EditorCoreTypes.LineNumber.ofZeroBased(bufferLineIdx),
      byteOffset: lineWrap.byte,
      characterOffset: lineWrap.character,
    };
  };
};

let numberOfLines = ({totalViewLines, _}) => {
  totalViewLines;
};

let maxLineLengthInPixels = ({maxLengthInPixels, _}) => maxLengthInPixels;
