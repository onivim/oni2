let bind2 = (a, b, f) =>
  switch (a, b) {
  | (Some(aVal), Some(bVal)) => f(aVal, bVal)
  | _ => None
  };

let flatMap = f =>
  fun
  | Some(x) => f(x)
  | None => None;

let map2 = (f, a, b) =>
  switch (a, b) {
  | (Some(aVal), Some(bVal)) => Some(f(aVal, bVal))
  | _ => None
  };

let iter2 = (f, a, b) => {
  switch (a, b) {
  | (Some(a), Some(b)) => f(a, b)
  | _ => ()
  };
};

let fallback = f =>
  fun
  | Some(_) as orig => orig
  | None => f();

let tap_none = f =>
  fun
  | Some(_) as v => v
  | None => {
      f();
      None;
    };

let iter_none = f =>
  fun
  | Some(_) => ()
  | None => f();

let flatten =
  fun
  | Some(x) => x
  | None => None;

let of_list =
  fun
  | [] => None
  | [hd, ..._] => Some(hd);

let zip = (a, b) =>
  switch (a, b) {
  | (Some(a), Some(b)) => Some((a, b))
  | _ => None
  };

let values = list => List.filter_map(Fun.id, list);

let toString = f =>
  fun
  | Some(v) => Printf.sprintf("Some(%s)", f(v))
  | None => "(None)";
