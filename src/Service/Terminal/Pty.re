open Oni_Core;

type t;

let start = (~env, ~cwd, ~rows, ~cols, ~cmd) => {};

let write = (_pty, data) => ();

let resize = (~rows, ~cols, _pty) => ();

let close = pty => ();
