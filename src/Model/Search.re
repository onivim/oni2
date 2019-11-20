open Oni_Core;

type t = {
  queryInput: string,
  query: string,
  cursorPosition: int,
  hits: list(Ripgrep.Match.t),
};

let initial = {queryInput: "", query: "", cursorPosition: 0, hits: []};