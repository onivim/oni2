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
        Range.create(
          ~start=
            Location.create(
              ~line=Index.fromZeroBased(currentPos),
              ~column=Index.zero,
            ),
          ~stop=
            Location.create(
              ~line=Index.fromZeroBased(currentPos),
              ~column=
                Index.fromZeroBased(
                  buffer
                  |> Buffer.getLine(currentPos)
                  |> BufferLine.lengthInBytes
                ),
            ),
        ),
        ...ranges^,
      ];

    incr(pos);
  };

  ranges^;
};

let getRangesForVisualSelection =
    (startLine, startColumn, endLine, endColumn, buffer) => {
  let pos = ref(startLine);
  let ranges = ref([]);

  while (pos^ <= endLine) {
    let currentPos = pos^;
    ranges :=
      [
        Range.create(
          ~start=
            Location.create(
              ~line=Index.fromZeroBased(currentPos),
              ~column=
                Index.fromZeroBased(
                  {
                    startLine == pos^ ? startColumn : 0;
                  },
                ),
            ),
          ~stop=
            Location.create(
              ~line=Index.fromZeroBased(currentPos),
              ~column=
                Index.fromZeroBased(
                  {
                    endLine == currentPos
                      ? endColumn + 1
                      : (
                      buffer
                      |> Buffer.getLine(currentPos)
                      |> BufferLine.lengthInBytes
                      )
                  },
                ),
            ),
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

    let bufferLength = buffer
    |> Buffer.getLine(currentPos)
    |> BufferLine.lengthInBytes;

    let newRange =
      if (startC < bufferLength) {
        Some(
          Range.create(
            ~start=
              Location.create(
                ~line=Index.fromZeroBased(currentPos),
                ~column=Index.fromZeroBased(startC),
              ),
            ~stop=
              Location.create(
                ~line=Index.fromZeroBased(currentPos),
                ~column=
                  Index.fromZeroBased(min(endColumn + 1, bufferLength)),
              ),
          ),
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
let getRanges: (VisualRange.t, Buffer.t) => list(Range.t) =
  (selection, buffer) => {
    let startLine = Index.toZeroBased(selection.range.start.line);
    let startCharacter = Index.toZeroBased(selection.range.start.column);

    let bufferLines = Buffer.getNumberOfLines(buffer);

    let endLine =
      min(Index.toZeroBased(selection.range.stop.line), bufferLines - 1);
    let endCharacter = Index.toZeroBased(selection.range.stop.column);

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
