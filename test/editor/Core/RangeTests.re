open TestFramework;

module Range = Oni_Core.Range;

open Helpers;

let constantMeasure = (length, _: int) => length;

let r = (startLine, startCharacter, endLine, endCharacter) =>
  Range.create(
    ~startLine=ZeroBasedIndex(startLine),
    ~startCharacter=ZeroBasedIndex(startCharacter),
    ~endLine=ZeroBasedIndex(endLine),
    ~endCharacter=ZeroBasedIndex(endCharacter),
    (),
  );

describe("Range", ({describe, _}) =>
  describe("explode", ({test, _}) => {
    test(
      "returns same range when start line/endline are the same", ({expect}) =>
      validateRanges(
        expect,
        Range.explode(constantMeasure(1), Range.zero),
        [Range.zero],
      )
    );

    test("simple two-line case", ({expect}) => {
      let range = r(1, 5, 2, 2);

      validateRanges(
        expect,
        Range.explode(constantMeasure(10), range),
        [r(1, 5, 1, 9), r(2, 0, 2, 2)],
      );
    });

    test("uses range function", ({expect}) => {
      let range = r(1, 5, 3, 2);

      validateRanges(
        expect,
        Range.explode(constantMeasure(10), range),
        [r(1, 5, 1, 9), r(2, 0, 2, 9), r(3, 0, 3, 2)],
      );
    });
  })
);
