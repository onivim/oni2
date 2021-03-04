open EditorCoreTypes;
open Oni_Core;

[@deriving (show({with_path: false}), yojson({strict: false}))]
type t = {
  range: OneBasedRange.t,
  text: string,
  // Deprecated - but some language servers, like the Java one,
  // still depend on it.
  rangeLength: int,
};

let create = (~rangeLength: int, ~range: CharacterRange.t, ~text: string, ()) => {
  range: OneBasedRange.ofRange(range),
  text,
  rangeLength,
};

let joinLines = (separator: string, lines: list(string)) => {
  String.concat(separator, lines);
};

let getRangeFromEdit = (bu: BufferUpdate.t) => {
  let newLines = Array.length(bu.lines);
  let isInsert =
    EditorCoreTypes.(
      newLines >= LineNumber.toZeroBased(bu.endLine)
      - LineNumber.toZeroBased(bu.startLine)
    );

  let startLine = EditorCoreTypes.LineNumber.toZeroBased(bu.startLine);
  let endLine =
    EditorCoreTypes.LineNumber.toZeroBased(bu.endLine) |> max(startLine);

  let range =
    EditorCoreTypes.(
      CharacterRange.{
        start:
          CharacterPosition.{
            line: LineNumber.ofZeroBased(startLine),
            character: CharacterIndex.zero,
          },
        stop:
          CharacterPosition.{
            line: LineNumber.ofZeroBased(endLine),
            character: CharacterIndex.zero,
          },
      }
    );

  (isInsert, range);
};

let getRangeLengthFromEdit =
    (~previousBuffer, ~eol: Eol.t, bu: BufferUpdate.t) => {
  let startLine = EditorCoreTypes.LineNumber.toZeroBased(bu.startLine);
  let endLine =
    EditorCoreTypes.LineNumber.toZeroBased(bu.endLine) |> max(startLine);

  let totalLines = Buffer.getNumberOfLines(previousBuffer);

  let eolSize = Eol.sizeInBytes(eol);
  let length = ref(0);
  for (lineNumber in startLine to min(endLine - 1, totalLines - 1)) {
    let lineLength =
      previousBuffer |> Buffer.getLine(lineNumber) |> BufferLine.lengthInBytes;
    length := lineLength + eolSize + length^;
  };

  length^;
};

let ofBufferUpdate =
    (~previousBuffer, bu: Oni_Core.BufferUpdate.t, eol: Eol.t) => {
      prerr_endline ("-- ofBufferUpdate - line length: " ++ string_of_int(Array.length(bu.lines)));
    prerr_endline ("--- Buffer Update: " ++ Oni_Core.BufferUpdate.toDebugString(bu));
  let (isInsert, range) = getRangeFromEdit(bu);
  let text = joinLines(Eol.toString(eol), bu.lines |> Array.to_list);
  let rangeLength = getRangeLengthFromEdit(~previousBuffer, ~eol, bu);

  let eolStr = Eol.toString(eol);
  // let text =
  //   isInsert
  //   || String.length(text) > 0
  //   && !Utility.StringEx.endsWith(~postfix=eolStr, text)
  //     ? text ++ eolStr : text;
  let text =
    isInsert ? text ++ eolStr : text;
    // || String.length(text) > 0
    // && !Utility.StringEx.endsWith(~postfix=eolStr, text)
    //   ? text ++ eolStr : text;

  {range: OneBasedRange.ofRange(range), text, rangeLength};
};
