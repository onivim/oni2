type expr =
  | Variable(string)
  | And(expr, expr)
  | Or(expr, expr)
  | Not(expr)
  | True
  | False;
