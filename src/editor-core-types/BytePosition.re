[@deriving show({with_path: false})]
type t = {
  line: LineNumber.t,
  byte: ByteIndex.t,
};

let equals = (a, b) => {
  a.line == b.line && a.byte == b.byte;
};

let (==) = equals;
