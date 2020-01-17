open RandomStuff;

let to_option =
  fun
  | Ok(v) => Some(v)
  | Error(_) => None;

let bind = f =>
  fun
  | Ok(v) => f(v)
  | Error(_) as err => err;

let map = f =>
  fun
  | Ok(v) => Ok(f(v))
  | Error(_) as err => err;

let default = (~value) =>
  fun
  | Ok(v) => v
  | Error(_) => value;

let exn =
  fun
  | Ok(v) => v
  | Error(msg) => raise(ResultError(msg));

let iter = f =>
  fun
  | Ok(v) => f(v)
  | Error(_) => ();

let iter_error = f =>
  fun
  | Ok(_) => ()
  | Error(err) => f(err);
