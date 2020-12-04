let guard = f =>
  try(Ok(f())) {
  | exn => Error(exn)
  };

let flatMap = f =>
  fun
  | Ok(x) => f(x)
  | Error(_) as err => err;

let tapError = f =>
  fun
  | Ok(_) as ok => ok
  | Error(msg) as err => {
      f(msg);
      err;
    };

let tap = f =>
  fun
  | Ok(v) as ok => {
      f(v);
      ok;
    }
  | Error(_) as err => err;

let value = (~default) =>
  fun
  | Ok(v) => v
  | Error(_) => default;

let map2 = (f, a, b) =>
  switch (a, b) {
  | (Ok(aVal), Ok(bVal)) => Ok(f(aVal, bVal))
  | (Ok(_), Error(_) as err) => err
  | (Error(_) as err, _) => err
  };
