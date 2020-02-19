type t =
  | Variable(string)
  | And(t, t)
  | Or(t, t)
  | Not(t)
  | True
  | False;

let evaluate: (t, string => bool) => bool;
let parse: string => result(t, string);
