/*
 * Selection
 *
 * Module for helpers in managing / rendering selection ranges
 * based on the visual selection state
 */

open Oni_Core.Types;

let getRangesForLinewiseSelection = (startLine, endLine, buffer) => {
  let pos = ref(startLine);
  let ranges = ref([]);

  while (pos^ <= endLine) {
    let currentPos = pos^;
    ranges :=
      [
        Range.create(
          ~startLine=ZeroBasedIndex(currentPos),
          ~startCharacter=ZeroBasedIndex(0),
          ~endLine=ZeroBasedIndex(currentPos),
          ~endCharacter=
            ZeroBasedIndex(Buffer.getLineLength(buffer, currentPos)),
          (),
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
          ~startLine=ZeroBasedIndex(currentPos),
          ~startCharacter=
            ZeroBasedIndex(
              {
                startLine == pos^ ? startColumn : 0;
              },
            ),
          ~endLine=ZeroBasedIndex(currentPos),
          ~endCharacter=
            ZeroBasedIndex(
              {
                endLine == currentPos
                  ? endColumn + 1 : Buffer.getLineLength(buffer, currentPos);
              },
            ),
          (),
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

  let (startC, endColumn) = (min(startC, endColumn), max(startC,endColumn));

  while (pos^ <= endLine) {
    let currentPos = pos^;

    let bufferLength = Buffer.getLineLength(buffer, currentPos);

    let newRange =
      if (startC < bufferLength) {
        Some(
          Range.create(
            ~startLine=ZeroBasedIndex(currentPos),
            ~startCharacter=ZeroBasedIndex(startC),
            ~endLine=ZeroBasedIndex(currentPos),
            ~endCharacter=ZeroBasedIndex(min(endColumn + 1, bufferLength)),
            (),
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
    let startLine = Index.toZeroBasedInt(selection.range.startPosition.line);
    let startCharacter =
      Index.toZeroBasedInt(selection.range.startPosition.character);

    let bufferLines = Buffer.getNumberOfLines(buffer);

    let endLine = min(Index.toZeroBasedInt(selection.range.endPosition.line), bufferLines - 1);
    let endCharacter =
      Index.toZeroBasedInt(selection.range.endPosition.character);

    switch (selection.mode) {
    | BlockwiseVisual =>
      getRangesForBlockSelection(
        startLine,
        startCharacter,
        endLine,
        endCharacter,
        buffer,
      )
    | LinewiseVisual =>
      getRangesForLinewiseSelection(startLine, endLine, buffer)
    | Visual =>
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
