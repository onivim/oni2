[@deriving show({with_path: false})]
type t = int; // 0-based internally

let zero = 0;

let fromZeroBased = i => i;
let fromOneBased = i =>
  if (i == 0) {
    raise(Invalid_argument("One-based index cannot be zero"));
  } else {
    i - 1;
  };

let toZeroBased = i => i;
let toOneBased = i => i + 1;

let equals = (==);

let (==) = (==);
let (>) = (>);
let (>=) = (>=);
let (<) = (<);
let (<=) = (<=);

let (+) = (+);
let (-) = (-);
let ( * ) = ( * );
let (/) = (/);

let toString = string_of_int;
