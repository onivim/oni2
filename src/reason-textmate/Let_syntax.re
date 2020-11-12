/*
 Let_syntax.re

 Helper for ppx_let for Result types
 */

let bind = (~f, v) => {
  switch (v) {
  | Ok(v) => f(v)
  | Error(e) => Error(e)
  };
};

let map = (~f) =>
  fun
  | Ok(v) => Ok(f(v))
  | Error(e) => Error(e);
