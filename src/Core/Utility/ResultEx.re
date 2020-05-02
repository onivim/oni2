let guard = f =>
  try(Ok(f())) {
  | exn => Error(exn)
  };

let flatMap = f =>
  fun
  | Ok(x) => f(x)
  | Error(_) as err => err;

let map2 = (f, a, b) => {
  switch (a, b) {
  | (Ok(vA), Ok(vB)) => Ok(f(vA, vB))
  | (Error(_) as errA, _) => errA
  | (_, Error(_) as errB) => errB
  };
};

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
