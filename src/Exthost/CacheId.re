open Oni_Core;

[@deriving show]
type t = int;

let decode = Json.Decode.int;

let encode = Json.Encode.int;
