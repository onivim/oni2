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
          ByteRange.{
            start:
              BytePosition.{
                line: LineNumber.ofZeroBased(currentPos),
                byte: ByteIndex.zero,
              },
            stop: {
              line: LineNumber.ofZeroBased(currentPos),
              byte:
                ByteIndex.ofInt(
                  buffer
                  |> Buffer.getLine(currentPos)
                  |> BufferLine.lengthInBytes,
                ),
            },
          }
        ),
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
    let startCharacterIdx = startLine == currentPos ? startColumn : 0;
    let endCharacterIdx =
      endLine == currentPos
        ? endColumn + 1
        : buffer |> Buffer.getLine(currentPos) |> BufferLine.lengthInBytes;
    ranges :=
      [
        EditorCoreTypes.(
          ByteRange.{
            start:
              BytePosition.{
                line: LineNumber.ofZeroBased(currentPos),
                byte: ByteIndex.ofInt(startCharacterIdx),
              },
            stop:
              BytePosition.{
                line: LineNumber.ofZeroBased(currentPos),
                byte: ByteIndex.ofInt(endCharacterIdx),
              },
          }
        ),
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
          ByteRange.{
            start:
              BytePosition.{
                line: EditorCoreTypes.LineNumber.ofZeroBased(currentPos),
                byte: ByteIndex.ofInt(startC),
              },
            stop:
              BytePosition.{
                line: EditorCoreTypes.LineNumber.ofZeroBased(currentPos),
                byte: ByteIndex.ofInt(min(endColumn + 1, bufferLength)),
              },
          },
        );
      } else {
        None;
      };

    ranges := [newRange, ...ranges^];

    incr(pos);
  };

  ranges^ |> List.filter_map(v => v);
};

/*
 * getRanges returns a list of Range.t in Buffer Space,
 * where selection highlights should be displayed
 */
let getRanges: (VisualRange.t, Buffer.t) => list(ByteRange.t) =
  (selection, buffer) => {
    let startLine =
      EditorCoreTypes.LineNumber.toZeroBased(selection.range.start.line);
    let startCharacter = ByteIndex.toInt(selection.range.start.byte);

    let bufferLines = Buffer.getNumberOfLines(buffer);

    let endLine =
      min(
        EditorCoreTypes.LineNumber.toZeroBased(selection.range.stop.line),
        bufferLines - 1,
      );
    let endCharacter = ByteIndex.toInt(selection.range.stop.byte);

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
