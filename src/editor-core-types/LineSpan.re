[@deriving show]
type t = {
  start: LineNumber.t,
  stop: LineNumber.t,
};

let zero = {start: LineNumber.zero, stop: LineNumber.zero};

let equals = (a, b) => LineNumber.(a.start == b.start && a.stop == b.stop);

let isSingleLine = span => span.start == span.stop;
