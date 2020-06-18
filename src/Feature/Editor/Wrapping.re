open Oni_Core;

type t = {
  wrap: WordWrap.t,
  buffer: EditorBuffer.t,
  wraps: array(list(WordWrap.lineWrap)),
};

module Internal = {
  let bufferToWraps = (~wrap, buffer) => {
    let bufferLineCount = EditorBuffer.numberOfLines(buffer);
    let wraps = Array.make(bufferLineCount, []);

    for (idx in 0 to bufferLineCount) {
      let line = EditorBuffer.line(idx, buffer);
      wraps[idx] = wrap(line);
    };
    wraps;
  };

  let bufferLineToViewLine = (bufferLine, {wraps, _}) => {
    let rec loop = (curr, idx) =>
      if (idx >= bufferLine) {
        curr;
      } else {
        loop(curr + List.length(wraps[idx]), idx + 1);
      };

    loop(0, 0);
  };
};

let make = (~wrap: Oni_Core.WordWrap.t, ~buffer) => {
  wrap,
  buffer,
  wraps: Internal.bufferToWraps(~wrap, buffer),
};

let update = (~update: Oni_Core.BufferUpdate.t, ~newBuffer, {wrap, _}) => {
  wrap,
  buffer: newBuffer,
  wraps: Internal.bufferToWraps(~wrap, newBuffer),
};

let bufferLineByteToViewLine = (~line, ~byteIndex: int, wrap) => {
  let startViewLine = Internal.bufferLineToViewLine(line, wrap);

  let viewLines = wrap.wraps[line];

  let rec loop = (idx, viewLines: list(WordWrap.lineWrap)) => {
    switch (viewLines) {
    | [] => idx
    | [hd] => idx
    | [hd, next, ...tail] =>
      if (byteIndex >= hd.byte && byteIndex < next.byte) {
        idx;
      } else {
        loop(idx + 1, [next, ...tail]);
      }
    };
  };

  let offset = loop(0, viewLines);
  startViewLine + offset;
};

let viewLineToBufferLine = (~line: int, t) => line;

let numberOfLines = wrapping => {
  let len = Array.length(wrapping.wraps);
  Internal.bufferLineToViewLine(len, wrapping);
};

let maxLineLength = ({buffer, _}) =>
  EditorBuffer.getEstimatedMaxLineLength(buffer);
