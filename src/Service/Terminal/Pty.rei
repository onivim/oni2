open Oni_Core;

type t;

let start:
  (
    ~env: list((string, string)),
    ~cwd: FpExp.t(FpExp.absolute),
    ~rows: int,
    ~height: int,
    ~cmd: string
  ) =>
  result(t, string);

let write: (t, string) => unit;

let resize: (~rows: int, ~cols: int, t) => unit;

let close: t => unit;
