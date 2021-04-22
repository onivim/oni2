[@deriving show]
type t = pri int;

let zero: t;

let fromZeroBased: int => t;
let fromOneBased: int => t;

let toZeroBased: t => int;
let toOneBased: t => int;

let equals: (t, t) => bool;

let (==): (t, t) => bool;
let (>): (t, t) => bool;
let (>=): (t, t) => bool;
let (<): (t, t) => bool;
let (<=): (t, t) => bool;

let (+): (t, int) => t;
let (-): (t, int) => t;
let ( * ): (t, int) => t;
let (/): (t, int) => t;

let toString: t => string;
