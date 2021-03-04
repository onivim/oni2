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
  let joined = String.concat(separator, lines);

  if (lines != []) {
    joined ++ separator;
  } else {
    joined;
  };
};

let getRangeFromEdit = (bu: BufferUpdate.t) => {
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

  range;
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
  let range = getRangeFromEdit(bu);
  let text = joinLines(Eol.toString(eol), bu.lines |> Array.to_list);
  let rangeLength = getRangeLengthFromEdit(~previousBuffer, ~eol, bu);

  {range: OneBasedRange.ofRange(range), text, rangeLength};
};
