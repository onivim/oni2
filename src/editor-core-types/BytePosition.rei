[@deriving show({with_path: false})]
type t = {
  line: LineNumber.t,
  byte: ByteIndex.t,
};

let zero: t;

let line: t => LineNumber.t;
let byte: t => ByteIndex.t;

let equals: (t, t) => bool;
let (==): (t, t) => bool;
