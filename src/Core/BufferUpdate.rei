module Index = Types.Index;

[@deriving show({with_path: false})]
type t = {
  // True if the update applies to the entire buffer, false otherwise
  isFull: bool,
  id: int,
  startLine: Index.t,
  endLine: Index.t,
  lines: array(string),
  version: int,
};

let create:
  (
    ~id: int=?,
    ~isFull: bool=?,
    ~startLine: Index.t,
    ~endLine: Index.t,
    ~lines: array(string),
    ~version: int,
    unit
  ) =>
  t;

let createFromZeroBasedIndices:
  (
    ~id: int=?,
    ~isFull: bool=?,
    ~startLine: int,
    ~endLine: int,
    ~lines: array(string),
    ~version: int,
    unit
  ) =>
  t;

let createFromOneBasedIndices:
  (
    ~id: int=?,
    ~isFull: bool=?,
    ~startLine: int,
    ~endLine: int,
    ~lines: array(string),
    ~version: int,
    unit
  ) =>
  t;
