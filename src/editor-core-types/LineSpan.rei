[@deriving show]
type t = {
  start: LineNumber.t,
  stop: LineNumber.t,
};

let zero: t;

let equals: (t, t) => bool;

let isSingleLine: t => bool;
