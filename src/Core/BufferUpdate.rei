open EditorCoreTypes;

[@deriving show({with_path: false})]
type t = {
  // True if the update applies to the entire buffer, false otherwise
  isFull: bool,
  id: int,
  startLine: LineNumber.t,
  endLine: LineNumber.t,
  lines: array(string),
  version: int,
};

let create:
  (
    ~id: int=?,
    ~isFull: bool=?,
    ~startLine: LineNumber.t,
    ~endLine: LineNumber.t,
    ~lines: array(string),
    ~version: int,
    unit
  ) =>
  t;

let toDebugString: t => string;
