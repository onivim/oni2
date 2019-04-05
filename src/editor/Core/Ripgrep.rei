type disposeFunction = unit => unit;

[@deriving show]
type t = {search: (string, list(string) => unit) => disposeFunction};

let make: string => t;
