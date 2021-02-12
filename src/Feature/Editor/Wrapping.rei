open EditorCoreTypes;
open Oni_Core;

type t;

type bufferPosition = {
  line: EditorCoreTypes.LineNumber.t,
  byteOffset: ByteIndex.t,
  characterOffset: CharacterIndex.t,
};

let make: (~wrap: Oni_Core.WordWrap.t, ~buffer: EditorBuffer.t) => t;

let update: (~update: BufferUpdate.t, ~newBuffer: EditorBuffer.t, t) => t;

let bufferBytePositionToViewLine: (~bytePosition: BytePosition.t, t) => int;
let viewLineToBufferPosition: (~line: int, t) => bufferPosition;

let numberOfLines: t => int;

let maxLineLengthInPixels: t => float;
