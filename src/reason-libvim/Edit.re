open EditorCoreTypes;

module Log = (val Timber.Log.withNamespace("SingleEdit"));

[@deriving show]
type t = {
  range: Range.t,
  text: option(string),
};

type editResult = {
  oldStartLine: Index.t,
  oldEndLine: Index.t,
  newLines: array(string),
};

let applyEdit = (~provider, edit) => {
  // For now, we assume the start line and end line are the same
  // this is safe for formatting operations, but we'll need to
  // improve this for more general edits (like code actions).
  let startLine = edit.range.start.line |> Index.toZeroBased;
  let endLine = edit.range.stop.line |> Index.toZeroBased;
  let startColumn = edit.range.start.column |> Index.toZeroBased;
  let endColumn = edit.range.stop.column |> Index.toZeroBased;

  let insertText = edit.text |> Option.value(~default="");

  if (startLine == endLine) {
    let lines' = Array.make(1, "");
    let maybeStr = provider(startLine);
    let line = edit.range.start.line;

    switch (maybeStr) {
    | None =>
      lines'[0] = insertText;
      Ok({oldStartLine: line, oldEndLine: line, newLines: lines'});
    | Some(lineStr) =>
      let len = String.length(lineStr);
      let before = String.sub(lineStr, 0, startColumn);
      let after = String.sub(lineStr, endColumn, len - endColumn);

      lines'[0] = before ++ insertText ++ after;
      Ok({oldStartLine: line, oldEndLine: line, newLines: lines'});
    };
  } else {
    Error("Edits across lines are not yet supported");
  };
};
