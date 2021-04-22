[@deriving show({with_path: false})]
type t = int;

let zero = 0;

let ofInt = Fun.id;
let toInt = Fun.id;

let (+) = (a, b) => a + b;
let (-) = (a, b) => a - b;
let ( * ) = (a, b) => a * b;
let (/) = (a, b) => a / b;
let (<) = (a, b) => a < b;
let (>) = (a, b) => a > b;
let (<=) = (a, b) => a <= b;
let (>=) = (a, b) => a >= b;

let max = max;

let compare = (a, b) => a - b;
