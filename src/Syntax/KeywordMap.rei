type t;

let empty: t;

let set:
  (~bufferId: int, ~scope: string, ~line: int, ~words: list(string), t) => t;

//let setBufferScope: (~bufferId: int, ~scope: string, t) => t;

let get: (~scope: string, t) => list(string);
