[@deriving show]
type t = int;

let zero = 0;

let ofZeroBased = lnum => lnum;
let ofOneBased = lnum => lnum - 1;

let toZeroBased = lnum => lnum;
let toOneBased = lnum => lnum + 1;

let (+) = (a, b) => a + b;
let (-) = (a, b) => a - b;

let equals = (a, b) => a == b;
let (==) = (a, b) => a == b;
let (<) = (a, b) => a < b;
let (>) = (a, b) => a > b;

let compare = (a, b) => a - b;
