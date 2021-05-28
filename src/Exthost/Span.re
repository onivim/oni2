open Oni_Core;
type t = {
  start: int,
  stop: int,
};

let encode = span => {
  Json.Encode.(
    obj([("start", span.start |> int), ("end", span.stop |> int)])
  );
};
