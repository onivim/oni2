open Oni_Core;

type t;

type bufferPosition = {
  line: int,
  byteOffset: int,
  characterOffset: int,
};

let make: (~wrap: Oni_Core.WordWrap.t, ~buffer: EditorBuffer.t) => t;

let update: (~update: BufferUpdate.t, ~newBuffer: EditorBuffer.t, t) => t;

let bufferLineByteToViewLine: (~line: int, ~byteIndex: int, t) => int;
let viewLineToBufferPosition: (~line: int, t) => bufferPosition;

let numberOfLines: t => int;
let maxLineLength: t => int;
