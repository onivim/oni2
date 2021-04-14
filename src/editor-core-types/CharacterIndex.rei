[@deriving show({with_path: false})]
// Zero-based UTF-8 character index
type t;

let zero: t;

let ofInt: int => t;
let toInt: t => int;

let (+): (t, int) => t;
let ( * ): (t, int) => t;
let (/): (t, int) => t;
let (-): (t, int) => t;
let (<): (t, t) => bool;
let (>): (t, t) => bool;
let (<=): (t, t) => bool;
let (>=): (t, t) => bool;

let max: (t, t) => t;

let compare: (t, t) => int;
