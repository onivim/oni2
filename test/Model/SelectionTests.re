open TestFramework;

module VisualRange = Oni_Core.VisualRange;
module Range = Oni_Core.Range;
module Buffer = Oni_Core.Buffer;
module Selection = Oni_Model.Selection;

open Oni_Core_Test.Helpers;

describe("Selection", ({test, _}) =>
  test("linewise: clamps range to buffer lines", ({expect}) => {
    let buffer = Buffer.ofLines([|"abc", "defg"|]);

    /* Visual range is one-based */
    let vr =
      VisualRange.create(
        ~startLine=1,
        ~startColumn=1,
        ~endLine=2,
        ~endColumn=5,
        ~mode=Vim.Types.Line,
        (),
      );

    let ranges = Selection.getRanges(vr, buffer) |> List.rev;

    expect.int(List.length(ranges)).toBe(2);

    let r0 = List.nth(ranges, 0);
    let r1 = List.nth(ranges, 1);

    let expectedR0 =
      Range.create(
        ~startLine=ZeroBasedIndex(0),
        ~endLine=ZeroBasedIndex(0),
        ~startCharacter=ZeroBasedIndex(0),
        ~endCharacter=ZeroBasedIndex(3),
        (),
      );

    let expectedR1 =
      Range.create(
        ~startLine=ZeroBasedIndex(1),
        ~endLine=ZeroBasedIndex(1),
        ~startCharacter=ZeroBasedIndex(0),
        ~endCharacter=ZeroBasedIndex(4),
        (),
      );

    validateRange(expect, r0, expectedR0);
    validateRange(expect, r1, expectedR1);
  })
);
