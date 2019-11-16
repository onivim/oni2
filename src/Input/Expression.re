type t =
  | Variable(string)
  | And(t, t)
  | Or(t, t)
  | Not(t)
  | True
  | False;

let rec show = (v: t) =>
  switch (v) {
  | Variable(s) => Printf.sprintf("Variable(%s)", s)
  | And(v1, v2) => Printf.sprintf("And(%s, %s)", show(v1), show(v2))
  | Or(v1, v2) => Printf.sprintf("Or(%s, %s)", show(v1), show(v2))
  | Not(v) => "!" ++ show(v)
  | True => "true"
  | False => "false"
  };

let of_list_or = items => {
  let rec build = (list, accum) => {
    switch (list) {
    | [] => accum
    | [hd, ...tail] => build(tail, Or(Variable(hd), accum))
    };
  };
  build(items, False);
};

let evaluate = (v: t, getValue) => {
  let rec eval = v =>
    switch (v) {
    | Variable(s) => getValue(s)
    | And(e1, e2) => eval(e1) && eval(e2)
    | Or(e1, e2) => eval(e1) || eval(e2)
    | Not(e) => !eval(e)
    | True => true
    | False => false
    };

  let ret = eval(v);

  Oni_Core.Log.debug(() =>
    Printf.sprintf(
      "Expression %s evaluated to: %s",
      show(v),
      ret ? "true" : "false",
    )
  );
  ret;
};
