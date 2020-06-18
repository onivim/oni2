open Oni_Core;

type t;

let make: (~wrap: Oni_Core.WordWrap.t, ~buffer: EditorBuffer.t) => t;

let update: (~update: BufferUpdate.t, ~newBuffer: EditorBuffer.t, t) => t;

let bufferLineByteToViewLine: (~line: int, ~byteIndex: int, t) => int;
let viewLineToBufferLine: (~line: int, t) => int;

let numberOfLines: t => int;
let maxLineLength: t => int;
