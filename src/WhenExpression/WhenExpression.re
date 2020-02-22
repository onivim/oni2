module Log = (val Oni_Core.Log.withNamespace("Oni2.WhenExpression"));

[@deriving show({with_path: false})]
type t =
  Types.expr =
    | Variable(string)
    | Eq(string, string)
    | Neq(string, string)
    | And(t, t)
    | Or(t, t)
    | Not(t)
    | True
    | False;

let evaluate = (expr, getValue) => {
  let rec eval =
    fun
    | Variable(name) =>
      switch (getValue(name)) {
      | `Bool(value) => value
      | _ => true
      }

    | Eq(variable, value) => getValue(variable) == `String(value)
    | Neq(variable, value) => getValue(variable) != `String(value)
    | And(e1, e2) => eval(e1) && eval(e2)
    | Or(e1, e2) => eval(e1) || eval(e2)
    | Not(e) => !eval(e)
    | True => true
    | False => false;

  let result = eval(expr);
  Log.tracef(m => m("Expression %s evaluated to: %b", show(expr), result));
  result;
};

let parse = str => {
  let lexbuf = Lexing.from_string(str);

  switch (When_parser.main(When_lexer.token, lexbuf)) {
  | expr => Ok(expr)
  | exception When_lexer.Error
  | exception When_parser.Error =>
    Error("Error parsing when expression: " ++ str)
  };
};
