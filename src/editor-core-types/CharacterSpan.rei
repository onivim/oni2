[@deriving show]
type t = {
  start: CharacterIndex.t,
  stop: CharacterIndex.t,
};

let zero: t;

let toRange: (~line: LineNumber.t, t) => CharacterRange.t;
