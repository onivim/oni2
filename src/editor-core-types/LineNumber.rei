[@deriving show({with_path: false})]
type t;

let zero: t;

let ofZeroBased: int => t;
let ofOneBased: int => t;
let toZeroBased: t => int;
let toOneBased: t => int;

let (+): (t, int) => t;
let (-): (t, int) => t;

let equals: (t, t) => bool;
let (==): (t, t) => bool;
let (>): (t, t) => bool;
let (<): (t, t) => bool;

let compare: (t, t) => int;
