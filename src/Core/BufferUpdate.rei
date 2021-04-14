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
  // [shouldAdjustCursorPosition] describes if the client should calculate a new cursor position based on the update.
  // For some updates, the cursor position has already been updated (ie, insert mode edits), so `shouldAdjustCursorPosition` would be `false`.
  // Other edits, like formatting, may not have adjusted the cursor position, and are relying on the client to make adjustments in the `true` case.
  shouldAdjustCursorPosition: bool,
};

let create:
  (
    ~id: int=?,
    ~isFull: bool=?,
    ~startLine: LineNumber.t,
    ~endLine: LineNumber.t,
    ~lines: array(string),
    ~version: int,
    ~shouldAdjustCursorPosition: bool,
    unit
  ) =>
  t;

let toDebugString: t => string;
