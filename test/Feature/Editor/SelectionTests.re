open EditorCoreTypes;
open TestFramework;

module VisualRange = Oni_Core.VisualRange;
module Buffer = Oni_Core.Buffer;
module Selection = Feature_Editor.Selection;

open Oni_Core_Test.Helpers;

describe("Selection", ({test, _}) =>
  test("linewise: clamps range to buffer lines", ({expect, _}) => {
    let buffer = Buffer.ofLines([|"abc", "defg"|]);

    /* Visual range is one-based */
    let vr =
      VisualRange.create(
        ~mode=Vim.Types.Line,
        ByteRange.{
          start: BytePosition.zero,
          stop:
            BytePosition.{
              line: LineNumber.(zero + 1),
              byte: ByteIndex.(zero + 4),
            },
        },
      );

    let ranges = Selection.getRanges(vr, buffer) |> List.rev;

    expect.int(List.length(ranges)).toBe(2);

    let r0 = List.nth(ranges, 0);
    let r1 = List.nth(ranges, 1);

    let expectedR0 =
      ByteRange.{
        start: BytePosition.zero,
        stop:
          BytePosition.{line: LineNumber.zero, byte: ByteIndex.(zero + 3)},
      };

    let expectedR1 =
      ByteRange.{
        start:
          BytePosition.{line: LineNumber.(zero + 1), byte: ByteIndex.zero},
        stop:
          BytePosition.{
            line: LineNumber.(zero + 1),
            byte: ByteIndex.(zero + 4),
          },
      };

    validateByteRange(expect, r0, expectedR0);
    validateByteRange(expect, r1, expectedR1);
  })
);
