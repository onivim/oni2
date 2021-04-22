[@deriving show({with_path: false})]
type t = {
  line: LineNumber.t,
  byte: ByteIndex.t,
};

let zero = {line: LineNumber.zero, byte: ByteIndex.zero};

let equals = (a, b) => {
  a.line == b.line && a.byte == b.byte;
};

let (==) = equals;

let line = ({line, _}) => line;
let byte = ({byte, _}) => byte;

let compare = (a, b) =>
  if (LineNumber.equals(a.line, b.line)) {
    ByteIndex.toInt(a.byte) - ByteIndex.toInt(b.byte);
  } else {
    LineNumber.toZeroBased(a.line) - LineNumber.toZeroBased(b.line);
  };
