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
        CharacterRange.{
          start:
            CharacterPosition.{
              line: LineNumber.zero,
              character: CharacterIndex.zero,
            },
          stop:
            CharacterPosition.{
              line: LineNumber.(zero + 1),
              character: CharacterIndex.(zero + 4),
            },
        },
      );

    let ranges = Selection.getRanges(vr, buffer) |> List.rev;

    expect.int(List.length(ranges)).toBe(2);

    let r0 = List.nth(ranges, 0);
    let r1 = List.nth(ranges, 1);

    let expectedR0 =
      CharacterRange.{
        start:
          CharacterPosition.{
            line: LineNumber.zero,
            character: CharacterIndex.zero,
          },
        stop:
          CharacterPosition.{
            line: LineNumber.(zero + 1),
            character: CharacterIndex.(zero + 3),
          },
      };

    let expectedR1 =
      CharacterRange.{
        start:
          CharacterPosition.{
            line: LineNumber.(zero + 1),
            character: CharacterIndex.zero,
          },
        stop:
          CharacterPosition.{
            line: LineNumber.(zero + 1),
            character: CharacterIndex.(zero + 4),
          },
      };

    validateCharacterRange(expect, r0, expectedR0);
    validateCharacterRange(expect, r1, expectedR1);
  })
);
