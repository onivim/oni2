[@deriving show]
type t =
  | Variable(string)
  | Eq(string, string)
  | Neq(string, string)
  | And(t, t)
  | Or(t, t)
  | Not(t)
  | True
  | False;

let evaluate: (t, string => [ | `Bool(bool) | `String(string)]) => bool;
let parse: string => result(t, string);
