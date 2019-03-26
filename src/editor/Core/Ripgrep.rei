[@deriving show]
type t = {search: string => list(string)};

let make: string => t;
