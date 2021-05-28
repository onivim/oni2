[@deriving show]
type t;

type marker =
  | Modified
  | Added
  | DeletedBefore
  | DeletedAfter
  | Unmodified;

let get: (~line: EditorCoreTypes.LineNumber.t, t) => marker;

let toArray: t => array(marker);

let generate: (~originalLines: array(string), Buffer.t) => t;
