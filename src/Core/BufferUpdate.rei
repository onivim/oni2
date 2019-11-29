module Index = Types.Index;

type t = {
  id: int,
  startCharacter: Index.t,
  startLine: Index.t,
  oldEndCharacter: Index.t,
  oldEndLine: Index.t,
  newEndCharacter: Index.t,
  newEndLine: Index.t,
  lines: array(string),
  version: int,
};

let create:
  (
    ~id: int,
    ~startCharacter: Index.t,
    ~startLine: Index.t,
    ~oldEndCharacter: Index.t,
    ~oldEndLine: Index.t,
    ~newEndCharacter: Index.t,
    ~newEndLine: Index.t,
    ~lines: array(string),
    ~version: int,
    unit
  ) =>
  t;

let toString: t => string;
