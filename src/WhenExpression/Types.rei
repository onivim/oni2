type expr =
  | Variable(string)
  | Eq(string, string)
  | Neq(string, string)
  | And(expr, expr)
  | Or(expr, expr)
  | Not(expr)
  | True
  | False;
