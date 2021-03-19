open EditorCoreTypes;

[@deriving show({with_path: false})]
type t = {
  // True if the update applies to the entire buffer, false otherwise
  isFull: bool,
  id: int,
  startLine: LineNumber.t,
  endLine: LineNumber.t,
  lines: [@opaque] array(string),
  version: int,
  shouldAdjustCursorPosition: bool,
};

let create =
    (
      ~id=0,
      ~isFull=false,
      ~startLine,
      ~endLine,
      ~lines,
      ~version,
      ~shouldAdjustCursorPosition,
      (),
    ) => {
  id,
  startLine,
  endLine,
  lines,
  version,
  isFull,
  shouldAdjustCursorPosition,
};

let toDebugString =
    (
      {
        isFull,
        version,
        startLine,
        endLine,
        lines,
        shouldAdjustCursorPosition,
        _,
      },
    ) => {
  let lineStr =
    lines
    |> Array.to_list
    |> List.mapi((idx, str) => Printf.sprintf("Line %d: |%s|", idx, str))
    |> String.concat("\n");
  Printf.sprintf(
    "Core.BufferUpdate - version %d (full: %b adjustCursor: %b) - startLine: %d endLine:%d\nLines:\n---\n%s\n---\n\n",
    version,
    isFull,
    shouldAdjustCursorPosition,
    startLine |> LineNumber.toZeroBased,
    endLine |> LineNumber.toZeroBased,
    lineStr,
  );
};
