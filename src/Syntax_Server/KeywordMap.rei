open Oni_Core;

type t;

let empty: t;

let add: (~bufferId: int, ~line: int, ~words: list(string), t) => t;

let setBufferScope: (~bufferId: int, ~scope: string, t) => t;

let getKeywords: (~scope: string, t) => list(string);
