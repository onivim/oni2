open EditorCoreTypes;

module Log = (val Kernel.Log.withNamespace("SingleEdit"));

[@deriving show]
type t = {
  range: Range.t,
  text: option(string),
};

let applyEdit = (~lines, edit) => {
  // For now, we assume the start line and end line are the same
  // this is safe for formatting operations, but we'll need to
  // improve this for more general edits (like code actions).
  let startLine = edit.range.start.line |> Index.toZeroBased;
  let endLine = edit.range.stop.line |> Index.toZeroBased;
  let startColumn = edit.range.start.column |> Index.toZeroBased;
  let endColumn = edit.range.stop.column |> Index.toZeroBased;

  let insertText = edit.text |> Option.value(~default="");

  if (startLine == endLine) {
    let lines' = Array.copy(lines);
    let lineStr = lines[startLine];
    let len = String.length(lineStr);
    let before = String.sub(lineStr, 0, startColumn);
    let after = String.sub(lineStr, endColumn, len - endColumn);

    lines'[startLine] = before ++ insertText ++ after;
    lines';
  } else {
    Log.warn("Edits across lines are not yet supported");
    Array.copy(lines);
  };
};
