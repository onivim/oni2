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
        ~mode=Vim.Types.Line,
        Range.create({
          start: Location.{line: Index.zero, column: Index.zero},
          start: Location.{line: Index.(zero + 1), column: Index.(zero + 4)},
        }),
      );

    let ranges = Selection.getRanges(vr, buffer) |> List.rev;

    expect.int(List.length(ranges)).toBe(2);

    let r0 = List.nth(ranges, 0);
    let r1 = List.nth(ranges, 1);

    let expectedR0 =
      Range.{
        start: Location.{line: Index.zero, column: Index.zero},
        stop: Location.{line: Index.zero, column: Index.(zero + 5)},
      };

    let expectedR1 =
      Range.create(
        ~start=
          Location.create(~line=Index.(zero + 1), ~column=Index.(zero + 1)),
        ~stop=Location.create(~line=Index.zero, ~column=Index.(zero + 4)),
      );

    validateRange(expect, r0, expectedR0);
    validateRange(expect, r1, expectedR1);
  })
);
