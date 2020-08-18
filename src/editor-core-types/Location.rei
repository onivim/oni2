[@deriving show]
type t = {
  line: Index.t,
  column: Index.t,
};

let create: (~line: Index.t, ~column: Index.t) => t;

let equals: (t, t) => bool;
let (==): (t, t) => bool;

let toString: t => string;
