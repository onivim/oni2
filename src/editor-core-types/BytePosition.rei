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

/**
 * [compare(a, b)] returns:
 * - 0 if a & b are equal
 * - a negative integer if a is less than b
 * - a positive integer if a is greater than b
 */
let compare: (t, t) => int;
