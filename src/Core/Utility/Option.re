let map = f =>
  fun
  | Some(x) => Some(f(x))
  | None => None;

let map2 = (f, a, b) =>
  switch (a, b) {
  | (Some(aVal), Some(bVal)) => Some(f(aVal, bVal))
  | _ => None
  };

let fallback = f =>
  fun
  | Some(_) as orig => orig
  | None => f();

let value = (~default) =>
  fun
  | Some(x) => x
  | None => default;

let iter = f =>
  fun
  | Some(x) => f(x)
  | None => ();

let iter2 = (f, a, b) => {
  switch (a, b) {
  | (Some(a), Some(b)) => f(a, b)
  | _ => ()
  };
};

let iter_none = f =>
  fun
  | Some(_) => ()
  | None => f();

let tap_none = f =>
  fun
  | Some(_) as v => v
  | None => {
      f();
      None;
    };

let some = x => Some(x);

let bind = f =>
  fun
  | Some(x) => f(x)
  | None => None;

let bind2 = (f, a, b) =>
  switch (a, b) {
  | (Some(aVal), Some(bVal)) => f(aVal, bVal)
  | _ => None
  };

let flatten =
  fun
  | Some(x) => x
  | None => None;

let join =
  fun
  | Some(x) => x
  | None => None;

let of_list =
  fun
  | [] => None
  | [hd, ..._] => Some(hd);

let toString = f =>
  fun
  | Some(v) => Printf.sprintf("Some(%s)", f(v))
  | None => "(None)";

let values: list(option('a)) => list('a) =
  items => List.filter_map(v => v, items);

let zip = (a, b) =>
  switch (a, b) {
  | (Some(a), Some(b)) => Some((a, b))
  | _ => None
  };
