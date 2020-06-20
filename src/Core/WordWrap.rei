type lineWrap = {
  byte: int,
  index: int,
};

type t = BufferLine.t => list(lineWrap);

// A wrapping implementation that never wraps
let none: t;

// A wrapping implementation that clamps the characters
// naively to a fixed column width.
let fixed: (~columns: int) => t;
