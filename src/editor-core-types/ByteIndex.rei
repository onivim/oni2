[@deriving show({with_path: false})]
type t;

let zero: t;

let ofInt: int => t;
let toInt: t => int;

let next: (string, t) => t;

let (+): (t, int) => t;
let (>): (t, t) => bool;
let (<): (t, t) => bool;
let (>=): (t, t) => bool;
let (<=): (t, t) => bool;
