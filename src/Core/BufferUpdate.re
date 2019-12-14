open EditorCoreTypes;

[@deriving show({with_path: false})]
type t = {
  // True if the update applies to the entire buffer, false otherwise
  isFull: bool,
  id: int,
  startLine: Index.t,
  endLine: Index.t,
  lines: [@opaque] array(string),
  version: int,
};

let create =
    (~id=0, ~isFull=false, ~startLine, ~endLine, ~lines, ~version, ()) => {
  id,
  startLine,
  endLine,
  lines,
  version,
  isFull,
};
