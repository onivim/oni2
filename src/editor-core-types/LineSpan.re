[@deriving show]
type t = {
  start: LineNumber.t,
  stop: LineNumber.t,
};

let zero = {start: LineNumber.zero, stop: LineNumber.zero};

module Internal = {
  let normalizeLines = span =>
    if (span.start > span.stop) {
      {start: span.stop, stop: span.start};
    } else {
      span;
    };
};

let equals = (a, b) => LineNumber.(a.start == b.start && a.stop == b.stop);

let isSingleLine = span => span.start == span.stop;
