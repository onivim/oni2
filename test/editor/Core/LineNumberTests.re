/* open Oni_Core; */
open TestFramework;

/* open Oni_Core.Types; */
module LineNumber = Oni_Core.LineNumber;

describe("LineNumber", ({test, _}) =>
  test("getNumberOfDigitsForLines", ({expect}) => {
    (LineNumber.getNumberOfDigitsForLines(0) |> expect.int).toBe(1);

    (LineNumber.getNumberOfDigitsForLines(1) |> expect.int).toBe(1);

    (LineNumber.getNumberOfDigitsForLines(9) |> expect.int).toBe(1);

    (LineNumber.getNumberOfDigitsForLines(10) |> expect.int).toBe(2);

    (LineNumber.getNumberOfDigitsForLines(99) |> expect.int).toBe(2);

    (LineNumber.getNumberOfDigitsForLines(100) |> expect.int).toBe(3);

    (LineNumber.getNumberOfDigitsForLines(999) |> expect.int).toBe(3);

    (LineNumber.getNumberOfDigitsForLines(100000) |> expect.int).toBe(6);

    (LineNumber.getNumberOfDigitsForLines(999999) |> expect.int).toBe(6);
  })
);
