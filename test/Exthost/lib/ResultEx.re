let tap_error = f =>
  fun
  | Ok(_) as v => v
  | Error(msg) as v => {
      f(msg);
      v;
    };
