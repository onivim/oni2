/*
 * Selection
 *
 * Module for helpers in managing / rendering selection ranges
 * based on the visual selection state
 */

open Oni_Core.Types;

/*
 * getRanges returns a list of Range.t in Buffer Space,
 * where selection highlights should be displayed
 */
let getRanges: (VisualRange.t, Buffer.t) => list(Range.t) =
  (selection: VisualRange.t, _buffer: Buffer.t) => {
    let startLine = Index.toZeroBasedInt(selection.range.startPos.line);
    /* let startCharacter = Index.toZeroBasedInt(selection.range.startPos.character); */

    let endLine = Index.toZeroBasedInt(selection.range.endPos.line);
    /* let endCharacter = Index.toZeroBasedInt(selection.range.endPos.character); */

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
  };
