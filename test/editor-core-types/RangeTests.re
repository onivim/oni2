open EditorCoreTypes;
open TestFramework;

open Helpers;

let constantMeasure = (length, _: Index.t) => length;

let createRange = (startLine, startColumn, stopLine, stopColumn) =>
  Range.create(
    ~start=
      Location.create(
        ~line=Index.fromZeroBased(startLine),
        ~column=Index.fromZeroBased(startColumn),
      ),
    ~stop=
      Location.create(
        ~line=Index.fromZeroBased(stopLine),
        ~column=Index.fromZeroBased(stopColumn),
      ),
  );

describe("Range", ({describe, _}) =>
  describe("explode", ({test, _}) => {
    test(
      "returns same range when start line/endline are the same",
      ({expect, _}) => {
      let emptyRange =
        Range.create(
          ~start=Location.create(~line=Index.zero, ~column=Index.zero),
          ~stop=Location.create(~line=Index.zero, ~column=Index.zero),
        );

      validateRanges(
        expect,
        Range.explode(constantMeasure(1), emptyRange),
        [emptyRange],
      );
    });

    test("simple two-line case", ({expect, _}) => {
      let range = createRange(1, 5, 2, 2);

      validateRanges(
        expect,
        Range.explode(constantMeasure(10), range),
        [createRange(1, 5, 1, 9), createRange(2, 0, 2, 2)],
      );
    });

    test("uses range function", ({expect, _}) => {
      let range = createRange(1, 5, 3, 2);

      validateRanges(
        expect,
        Range.explode(constantMeasure(10), range),
        [
          createRange(1, 5, 1, 9),
          createRange(2, 0, 2, 9),
          createRange(3, 0, 3, 2),
        ],
      );
    });
  })
);
