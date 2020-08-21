[@deriving show({with_path: false})]
type t = {
    line: LineNumber.t,
    
    byte: ByteIndex.t,
};

let equals: (t, t) => bool;
let (==): (t, t) => bool;
