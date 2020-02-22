[@deriving show]
type t =
  | Defined(string)
  | Eq(string, value)
  | Neq(string, value)
  | And(t, t)
  | Or(t, t)
  | Not(t)
  | Value(value)

and value =
  | String(string)
  | True
  | False;

let evaluate: (t, string => value) => bool;
let parse: string => result(t, string);
