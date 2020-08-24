open EditorCoreTypes;

type lineWrap = {
  byte: ByteIndex.t,
  character: CharacterIndex.t,
};

type t = BufferLine.t => list(lineWrap);

// A wrapping implementation that never wraps
let none: t;

// A wrapping implementation that clamps the characters
// naively to a fixed pixel width
let fixed: (~pixels: float) => t;
