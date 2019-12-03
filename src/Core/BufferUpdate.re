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

let create =
    (~id=0, ~isFull=false, ~startLine, ~endLine, ~lines, ~version, ()) => {
  let ret: t = {id, startLine, endLine, lines, version, isFull};
  ret;
};

let createFromZeroBasedIndices =
    (
      ~id=0,
      ~isFull=false,
      ~startLine: int,
      ~endLine: int,
      ~lines,
      ~version,
      (),
    ) => {
  let ret: t = {
    isFull,
    id,
    startLine: Index.ZeroBasedIndex(startLine),
    endLine: Index.ZeroBasedIndex(endLine),
    lines,
    version,
  };
  ret;
};

let createFromOneBasedIndices =
    (
      ~id=0,
      ~isFull=false,
      ~startLine: int,
      ~endLine: int,
      ~lines,
      ~version,
      (),
    ) => {
  let ret: t = {
    isFull,
    id,
    startLine: Index.OneBasedIndex(startLine),
    endLine: Index.OneBasedIndex(endLine),
    lines,
    version,
  };
  ret;
};
