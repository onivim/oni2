open Oni_Core;

type t;

let start:
  (
    ~setup: Setup.t,
    ~env: list((string, string)),
    ~cwd: string,
    ~rows: int,
    ~cols: int,
    ~cmd: string,
    ~arguments: list(string),
    ~onData: string => unit,
    ~onPidChanged: int => unit,
    ~onTitleChanged: string => unit,
    ~onExit: (~exitCode: int) => unit
  ) =>
  result(t, string);

let write: (t, string) => unit;

let resize: (~rows: int, ~cols: int, t) => unit;

let close: t => unit;
