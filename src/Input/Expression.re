type t =
| Variable(string)
| And(t, t)
| Or(t, t)
| Not(t);


let rec show = (v: t) => switch(v) {
| Variable(s) => Printf.sprintf("Variable(%s)", s);
| And(v1, v2) => Printf.sprintf("And(%s, %s)", show(v1), show(v2))
| Or(v1, v2) => Printf.sprintf("Or(%s, %s)", show(v1), show(v2))
| Not(v) => "!" ++ show(v);
}

