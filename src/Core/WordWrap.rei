open EditorCoreTypes;

type lineWrap = {
  byte: ByteIndex.t,
  character: CharacterIndex.t,
};

// A [wrapCalculator] of type [t] returns a list of wraps, as well
// as a float value - the float being the total size of the text, in pixels.
type t = BufferLine.t => (array(lineWrap), float);

// A wrapping implementation that never wraps
let none: t;

// A wrapping implementation that clamps the characters
// naively to a fixed pixel width
let fixed: (~simpleMeasurement: bool=?, ~pixels: float) => t;
