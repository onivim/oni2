type disposeFunction = unit => unit;

[@deriving show]
type t = {
  search:
    (string, string, list(string) => unit, unit => unit) => disposeFunction,
};

let getRunCount: unit => int;
let getCompletedCount: unit => int;

let make: string => t;
