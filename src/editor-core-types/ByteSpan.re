[@deriving show]
type t = {
  start: ByteIndex.t,
  stop: ByteIndex.t,
};

let zero = {start: ByteIndex.zero, stop: ByteIndex.zero};

let toRange = (~line as l, span) => {
  let {start: startByte, stop: stopByte} = span;
  ByteRange.{
    start: BytePosition.{line: l, byte: startByte},
    stop: BytePosition.{line: l, byte: stopByte},
  };
};

let ofRange = (range: ByteRange.t) =>
  if (!LineNumber.equals(range.start.line, range.stop.line)) {
    None;
  } else {
    Some({start: range.start.byte, stop: range.stop.byte});
  };
