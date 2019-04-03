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

    print_endline ("GET RANGES FOR LINEWISE SELECTION");

    while (pos^ <= endLine) {

        let currentPos = pos^;
        ranges := [Range.create(
            ~startLine=ZeroBasedIndex(currentPos),
            ~startColumn=ZeroBasedIndex(0),
            ~endLine=ZeroBasedIndex(currentPos),
            ~endColumn=ZeroBasedIndex(Buffer.getLineLength(buffer, currentPos)),
            (),
        ), ...ranges^]

        incr(pos);
    }
    
    ranges^
};

let getRangesForVisualSelection = (startLine, startColumn, endLine, endColumn, buffer) => {
    let pos = ref(startLine);
    let ranges = ref([]);

    while (pos^ <= endLine) {

        let currentPos = pos^;
        ranges := [Range.create(
            ~startLine=ZeroBasedIndex(currentPos),
            ~startColumn=ZeroBasedIndex({startLine == pos^ ? startColumn : 0}),
            ~endLine=ZeroBasedIndex(currentPos),
            ~endColumn=ZeroBasedIndex({endLine == currentPos ? endColumn + 1 : Buffer.getLineLength(buffer, currentPos)}),
            (),
        ), ...ranges^]

        incr(pos);
    }
    
    ranges^
};

/*
 * getRanges returns a list of Range.t in Buffer Space,
 * where selection highlights should be displayed
 */
let getRanges: (VisualRange.t, Buffer.t) => list(Range.t) =
  (selection, buffer) => {
    let startLine = Index.toZeroBasedInt(selection.range.startPos.line);
    let startCharacter = Index.toZeroBasedInt(selection.range.startPos.character);

    let endLine = Index.toZeroBasedInt(selection.range.endPos.line);
    let endCharacter = Index.toZeroBasedInt(selection.range.endPos.character);

    print_endline ("MODE: " ++ VisualRange.show(selection));

    switch (selection.mode) {
    | LinewiseVisual => getRangesForLinewiseSelection(startLine, endLine, buffer)
    | Visual => getRangesForVisualSelection(startLine, startCharacter, endLine, endCharacter, buffer)
    | _ => {
    if (startLine == endLine) {
      [
        Range.create(
          ~startLine=ZeroBasedIndex(startLine),
          ~startColumn=ZeroBasedIndex(1),
          /* ~startColumn=ZeroBasedIndex(startCharacter), */
          ~endLine=ZeroBasedIndex(endLine),
          ~endColumn=ZeroBasedIndex(5),
          (),
        ),
      ];
    } else {
      [];
    };
    }
    };

  };
