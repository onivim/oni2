open EditorCoreTypes;

type lineWrap = {
  byte: ByteIndex.t,
  character: CharacterIndex.t,
};

type t = BufferLine.t => array(lineWrap);

// A wrapping implementation that never wraps
let none: t;

// A wrapping implementation that clamps the characters
// naively to a fixed pixel width
let fixed: (~simpleMeasurement: bool=?, ~pixels: float) => t;
