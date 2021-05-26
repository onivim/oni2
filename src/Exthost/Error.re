open Oni_Core;

[@deriving show]
type t = {
  name: string,
  message: string,
  stack: string,
};

let encode = err => {
  Json.Encode.(
    obj([
      ("$isError", bool(true)),
      ("name", string(err.name)),
      ("message", string(err.message)),
      ("stack", string(err.stack)),
    ])
  );
};
