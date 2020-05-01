open EditorCoreTypes;
open Oni_Core;

[@deriving (show({with_path: false}), yojson({strict: false}))]
type t = {
  range: OneBasedRange.t,
  text: string,
};

let create = (~range: Range.t, ~text: string, ()) => {
  range: OneBasedRange.ofRange(range),
  text,
};

let joinLines = (separator: string, lines: list(string)) => {
  String.concat(separator, lines);
};

let getRangeFromEdit = (bu: BufferUpdate.t) => {
  let newLines = Array.length(bu.lines);
  let isInsert =
    newLines >= Index.toZeroBased(bu.endLine)
    - Index.toZeroBased(bu.startLine);

  let startLine = Index.toZeroBased(bu.startLine);
  let endLine = Index.toZeroBased(bu.endLine) |> max(startLine);

  let range =
    Range.{
      start:
        Location.{line: Index.fromZeroBased(startLine), column: Index.zero},
      stop:
        Location.{line: Index.fromZeroBased(endLine), column: Index.zero},
    };

  (isInsert, range);
};

let ofBufferUpdate = (bu: Oni_Core.BufferUpdate.t, eol: Eol.t) => {
  let (isInsert, range) = getRangeFromEdit(bu);
  let text = joinLines(Eol.toString(eol), bu.lines |> Array.to_list);

  let text = isInsert ? text ++ Eol.toString(eol) : text;

  {range: OneBasedRange.ofRange(range), text};
};
