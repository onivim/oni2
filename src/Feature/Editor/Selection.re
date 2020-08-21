/*
 * Selection
 *
 * Module for helpers in managing / rendering selection ranges
 * based on the visual selection state
 */

open EditorCoreTypes;
open Oni_Core;

let getRangesForLinewiseSelection = (startLine, endLine, buffer) => {
  let pos = ref(startLine);
  let ranges = ref([]);

  while (pos^ <= endLine) {
    let currentPos = pos^;
    ranges :=
      [
        EditorCoreTypes.(
        CharacterRange.{
          start: CharacterPosition.{
            line: LineNumber.ofZeroBased(currentPos),
            character: CharacterIndex.zero,
          },
          stop: {
            line: LineNumber.ofZeroBased(currentPos),
            character: CharacterIndex.ofInt(
                  buffer
                  |> Buffer.getLine(currentPos)
                  |> BufferLine.lengthInBytes,
            )
          }
        }),
        ...ranges^,
      ];

    incr(pos);
  };

  ranges^;
};

let getRangesForVisualSelection =
    (startLine: int, startColumn: int, endLine: int, endColumn: int, buffer) => {
  let pos = ref(startLine);
  let ranges = ref([]);

  while (pos^ <= endLine) {
    let currentPos = pos^;
    let startCharacterIdx = startLine == currentPos ? startColumn : 0
    let endCharacterIdx = 
                    endLine == currentPos
                      ? endColumn + 1
                      : buffer
                        |> Buffer.getLine(currentPos)
                        |> BufferLine.lengthInBytes
    ranges :=
      [
        EditorCoreTypes.(
        CharacterRange.{
            start: CharacterPosition.{
              line: LineNumber.ofZeroBased(currentPos),
              character: CharacterIndex.ofInt(startCharacterIdx),
            },
            stop: CharacterPosition.{
              line: LineNumber.ofZeroBased(currentPos),
              character: CharacterIndex.ofInt(endCharacterIdx),
            },
          }),
        ...ranges^,
      ];

    incr(pos);
  };

  ranges^;
};

let getRangesForBlockSelection =
    (startLine, startC, endLine, endColumn, buffer) => {
  let pos = ref(startLine);
  let ranges = ref([]);

  let (startC, endColumn) = (
    min(startC, endColumn),
    max(startC, endColumn),
  );

  while (pos^ <= endLine) {
    let currentPos = pos^;

    let bufferLength =
      buffer |> Buffer.getLine(currentPos) |> BufferLine.lengthInBytes;

    let newRange =
      if (startC < bufferLength) {
        Some(
          CharacterRange.{
            start: CharacterPosition.{
              line: EditorCoreTypes.LineNumber.ofZeroBased(currentPos),
              character: CharacterIndex.ofInt(startC),
            },
            stop: CharacterPosition.{
              line: EditorCoreTypes.LineNumber.ofZeroBased(currentPos),
              character: CharacterIndex.ofInt(min(endColumn + 1, bufferLength)),
            }
          },
        );
      } else {
        None;
      };

    ranges := [newRange, ...ranges^];

    incr(pos);
  };

  let hasValue = v =>
    switch (v) {
    | Some(_) => true
    | None => false
    };

  let getValue = v =>
    switch (v) {
    | Some(v) => v
    | None => failwith("Should've been filtered out")
    };

  ranges^ |> List.filter(hasValue) |> List.map(getValue);
};

/*
 * getRanges returns a list of Range.t in Buffer Space,
 * where selection highlights should be displayed
 */
let getRanges: (VisualRange.t, Buffer.t) => list(CharacterRange.t) =
  (selection, buffer) => {
    let startLine = EditorCoreTypes.LineNumber.toZeroBased(selection.range.start.line);
    let startCharacter = CharacterIndex.toInt(selection.range.start.character);

    let bufferLines = Buffer.getNumberOfLines(buffer);

    let endLine =
      min(EditorCoreTypes.LineNumber.toZeroBased(selection.range.stop.line), bufferLines - 1);
    let endCharacter = CharacterIndex.toInt(selection.range.stop.character);

    switch (selection.mode) {
    | Vim.Types.Block =>
      getRangesForBlockSelection(
        startLine,
        startCharacter,
        endLine,
        endCharacter,
        buffer,
      )
    | Vim.Types.Line =>
      getRangesForLinewiseSelection(startLine, endLine, buffer)
    | Vim.Types.Character =>
      getRangesForVisualSelection(
        startLine,
        startCharacter,
        endLine,
        endCharacter,
        buffer,
      )
    | _ => []
    };
  };
