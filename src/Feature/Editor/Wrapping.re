open EditorCoreTypes;
open Oni_Core;

type bufferPosition = {
  line: EditorCoreTypes.LineNumber.t,
  byteOffset: ByteIndex.t,
  characterOffset: CharacterIndex.t,
};

type t = {
  wrap: WordWrap.t,
  buffer: EditorBuffer.t,
  wraps: array(list(WordWrap.lineWrap)),
  // Map of buffer line index -> view line index
  bufferLineToViewLineCache: IntMap.t(int),
  // Map of view line index -> buffer index
  viewLineToBufferCache: IntMap.t(int),
  totalViewLines: int,
};

module Internal = {
  let bufferToWraps = (~wrap, buffer) => {
    let bufferLineCount = EditorBuffer.numberOfLines(buffer);
    let wraps = Array.make(bufferLineCount, []);

    for (idx in 0 to bufferLineCount - 1) {
      let line = EditorBuffer.line(idx, buffer);
      wraps[idx] = wrap(line);
    };
    wraps;
  };

  let recalculateCaches = (~wraps: array(list(WordWrap.lineWrap))) => {
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
        let wrapCount = wraps[idx] |> List.length;
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
};

let make = (~wrap: Oni_Core.WordWrap.t, ~buffer) => {
  let wraps = Internal.bufferToWraps(~wrap, buffer);
  let (bufferLineToViewLineCache, viewLineToBufferCache, totalViewLines) =
    Internal.recalculateCaches(~wraps);
  {
    wrap,
    buffer,
    wraps,
    bufferLineToViewLineCache,
    viewLineToBufferCache,
    totalViewLines,
  };
};

let update = (~update as _: Oni_Core.BufferUpdate.t, ~newBuffer, {wrap, _}) => {
  let wraps = Internal.bufferToWraps(~wrap, newBuffer);
  let (bufferLineToViewLineCache, viewLineToBufferCache, totalViewLines) =
    Internal.recalculateCaches(~wraps);
  {
    wrap,
    buffer: newBuffer,
    wraps,
    bufferLineToViewLineCache,
    viewLineToBufferCache,
    totalViewLines,
  };
};

let bufferBytePositionToViewLine = (~bytePosition: BytePosition.t, wrap) => {
  let line = EditorCoreTypes.LineNumber.toZeroBased(bytePosition.line);
  let startViewLine = Internal.bufferLineToViewLine(line, wrap);
  let byteIndex = bytePosition.byte;

  let viewLines = wrap.wraps[line];

  let rec loop = (idx, viewLines: list(WordWrap.lineWrap)) => {
    switch (viewLines) {
    | [] => idx
    | [_] => idx
    | [hd, next, ...tail] =>
      if (ByteIndex.(byteIndex >= hd.byte && byteIndex < next.byte)) {
        idx;
      } else {
        loop(idx + 1, [next, ...tail]);
      }
    };
  };

  let offset = loop(0, viewLines);
  startViewLine + offset;
};

let viewLineToBufferPosition = (~line: int, wrapping) => {
  let bufferLineIdx = Internal.viewLineToBufferLine(line, wrapping);
  let startViewLine = Internal.bufferLineToViewLine(bufferLineIdx, wrapping);

  let wraps = wrapping.wraps[bufferLineIdx];

  let lineWrap =
    List.nth_opt(wraps, line - startViewLine)
    // This shouldn't ever happen, if our caches are correct.
    |> Option.value(
         ~default=
           WordWrap.{byte: ByteIndex.zero, character: CharacterIndex.zero},
       );

  {
    line: EditorCoreTypes.LineNumber.ofZeroBased(bufferLineIdx),
    byteOffset: lineWrap.byte,
    characterOffset: lineWrap.character,
  };
};

let numberOfLines = ({totalViewLines, _}) => {
  totalViewLines;
};

let maxLineLength = ({buffer, _}) =>
  EditorBuffer.getEstimatedMaxLineLength(buffer);
