[@deriving show]
type t = int;

let zero = 0;

let ofZeroBased = lnum => lnum
let ofOneBased = lnum => lnum - 1;

let toZeroBased = lnum => lnum;
let toOneBased = lnum => lnum + 1;
