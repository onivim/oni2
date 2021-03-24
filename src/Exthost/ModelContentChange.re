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
  String.concat(
    separator,
    lines,
    // if (lines != []) {
    //   joined ++ separator;
    // } else {
    //   joined;
    // };
  );
};

let getRangeFromEdit = (~previousBuffer, bu: BufferUpdate.t) => {
  open EditorCoreTypes;
  let startLine = LineNumber.toZeroBased(bu.startLine);

  let _previousBufferLastLine = Buffer.getNumberOfLines(previousBuffer);
  let endLine = LineNumber.toZeroBased(bu.endLine) - 1 |> max(startLine);
  let start =
    CharacterPosition.{
      line: LineNumber.ofZeroBased(startLine),
      character: CharacterIndex.zero,
    };
  let stop =
    if (endLine < Buffer.getNumberOfLines(previousBuffer)) {
      let previousBufferLastLineLength =
        previousBuffer |> Buffer.getLine(endLine) |> BufferLine.lengthSlow;
      CharacterPosition.{
        line: LineNumber.ofZeroBased(endLine),
        character: CharacterIndex.ofInt(previousBufferLastLineLength),
      };
    } else {
      CharacterPosition.{
        line: LineNumber.ofZeroBased(endLine),
        character: CharacterIndex.zero,
      };
    };

  let range = CharacterRange.{start, stop};

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

let getLineLength = (~buffer, ~eol, ~includeEol, line) => {
  Buffer.rawLine(line, buffer)
  |> Option.map(line => {
       let length = String.length(line);

       if (includeEol) {
         length + Eol.sizeInBytes(eol);
       } else {
         length;
       };
     })
  |> Option.value(~default=0);
};

let lastPositionOfBuffer = buffer => {
  let lineCount = Buffer.getNumberOfLines(buffer);
  if (lineCount == 0) {
    CharacterPosition.zero;
  } else {
    let line = EditorCoreTypes.LineNumber.ofZeroBased(lineCount - 1);
    let length =
      getLineLength(~buffer, ~includeEol=false, ~eol=Eol.default, line);
    CharacterPosition.{line, character: CharacterIndex.ofInt(length)};
  };
};

let ofMinimalUpdate = (~previousBuffer, ~eol, mu: MinimalUpdate.update) => {
  open EditorCoreTypes;
  let eolStr = Eol.toString(eol);
  let (range, text, rangeLength) =
    MinimalUpdate.(
      switch (mu) {
      | Modified({line, original, updated}) =>
        let text = updated;
        let rangeLength = String.length(original);

        let start = CharacterPosition.{line, character: CharacterIndex.zero};
        let stop =
          CharacterPosition.{
            line,
            character: CharacterIndex.ofInt(rangeLength),
          };
        let range = CharacterRange.{start, stop};
        (range, text, rangeLength);
      | Deleted({startLine, stopLine}) =>
        let isThroughLastLine =
          LineNumber.toZeroBased(stopLine)
          >= Buffer.getNumberOfLines(previousBuffer);
        let text = "";
        let start =
          CharacterPosition.{line: startLine, character: CharacterIndex.zero};

        let stop =
          isThroughLastLine
            ? lastPositionOfBuffer(previousBuffer)
            : CharacterPosition.{
                line: stopLine,
                character: CharacterIndex.zero,
              };

        let range = CharacterRange.{start, stop};
        // TODO
        let rangeLength = 0;
        (range, text, rangeLength);
      | Added({beforeLine, lines}) =>
        let isBeforeFirstLine = beforeLine == LineNumber.zero;
        let isThroughLastLine =
          LineNumber.toZeroBased(beforeLine)
          >= Buffer.getNumberOfLines(previousBuffer);
        let combinedText = String.concat(eolStr, Array.to_list(lines));
        let text =
          if (isBeforeFirstLine && isThroughLastLine) {
            combinedText;
          } else if (isBeforeFirstLine) {
            combinedText ++ eolStr;
          } else {
            eolStr ++ combinedText;
          };
        let position =
          isBeforeFirstLine
            ? CharacterPosition.{
                line: beforeLine,
                character: CharacterIndex.zero,
              }
            : {
              let previousLine = LineNumber.(beforeLine - 1);
              let previousLineLength =
                getLineLength(
                  ~buffer=previousBuffer,
                  ~eol,
                  ~includeEol=false,
                  previousLine,
                );
              CharacterPosition.{
                line: previousLine,
                character: CharacterIndex.ofInt(previousLineLength),
              };
            };
        let rangeLength = 0;
        let range = CharacterRange.{start: position, stop: position};
        (range, text, rangeLength);
      }
    );

  {range: OneBasedRange.ofRange(range), text, rangeLength};
};

let ofMinimalUpdates = (~previousBuffer, ~eol, updates) => {
  updates |> List.map(ofMinimalUpdate(~previousBuffer, ~eol));
};
