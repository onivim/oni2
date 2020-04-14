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

let map3 = (f, a, b, c) =>
  switch (a, b, c) {
  | (Some(aVal), Some(bVal), Some(cVal)) => Some(f(aVal, bVal, cVal))
  | _ => None
  };

let iter2 = (f, a, b) => {
  switch (a, b) {
  | (Some(a), Some(b)) => f(a, b)
  | _ => ()
  };
};

let or_ = other =>
  fun
  | Some(_) as orig => orig
  | None => other;

let or_lazy = f =>
  fun
  | Some(_) as orig => orig
  | None => f();

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

let value_lazy = (~default) =>
  fun
  | Some(v) => v
  | None => default();

let toString = f =>
  fun
  | Some(v) => Printf.sprintf("Some(%s)", f(v))
  | None => "(None)";
