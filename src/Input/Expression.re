type t =
  | Variable(string)
  | And(t, t)
  | Or(t, t)
  | Not(t);

let rec show = (v: t) =>
  switch (v) {
  | Variable(s) => Printf.sprintf("Variable(%s)", s)
  | And(v1, v2) => Printf.sprintf("And(%s, %s)", show(v1), show(v2))
  | Or(v1, v2) => Printf.sprintf("Or(%s, %s)", show(v1), show(v2))
  | Not(v) => "!" ++ show(v)
  };

let parse = str => {
  let lexbuf = Lexing.from_string(str /*"abc && (def || !ghi)"*/);
  switch (When_parser.main(When_lexer.token, lexbuf)) {
  | exception When_lexer.Error => Error("Error parsing when binding: " ++ str)
  | exception When_parser.Error =>
    Error("Error parsing when binding: " ++ str)
  | v => Ok(v)
  };
};

let evaluate = (v: t, getValue) => {
  let rec f = v =>
    switch (v) {
    | Variable(s) => getValue(s)
    | And(e1, e2) => f(e1) && f(e2)
    | Or(e1, e2) => f(e1) || f(e2)
    | Not(e) => !f(e)
    };

  f(v);
};
