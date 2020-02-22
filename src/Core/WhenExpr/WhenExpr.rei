module Value: {
  [@deriving show]
  type t =
    | String(string)
    | True
    | False;
};

[@deriving show]
type t =
  | Defined(string)
  | Eq(string, Value.t)
  | Neq(string, Value.t)
  | And(list(t))
  | Or(list(t))
  | Not(t)
  | Value(Value.t);

let evaluate: (t, string => Value.t) => bool;
let parse: string => t;
