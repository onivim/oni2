[@deriving show]
type t = {
  start: ByteIndex.t,
  stop: ByteIndex.t,
};

let zero: t;

let toRange: (~line: LineNumber.t, t) => ByteRange.t;

let ofRange: ByteRange.t => option(t);
