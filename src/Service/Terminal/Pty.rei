open Oni_Core;

type t;

let start:
  (
    ~env: list((string, string)),
    ~cwd: string,
    ~rows: int,
    ~cols: int,
    ~cmd: string,
    string => unit
  ) =>
  result(t, string);

let write: (t, string) => unit;

let resize: (~rows: int, ~cols: int, t) => unit;

let close: t => unit;
