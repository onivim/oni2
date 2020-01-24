let some = x => Some(x);

let value = (~default) =>
  fun
  | Some(x) => x
  | None => default;

let bind = (o, f) =>
  switch (o) {
  | Some(x) => f(x)
  | None => None
  };

let join =
  fun
  | Some(x) => x
  | None => None;

let map = f =>
  fun
  | Some(x) => Some(f(x))
  | None => None;

let iter = f =>
  fun
  | Some(x) => f(x)
  | None => ();
