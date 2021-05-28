open EditorCoreTypes;
open Oni_Core;
open TestFramework;

let makeLine = BufferLine.make(~measure=_ => 1.0);

let makeLineWithWideTabs =
  BufferLine.make(~measure=uchar =>
    if (Uchar.equal(uchar, Uchar.of_char('\t'))) {
      2.0;
    } else {
      1.0;
    }
  );

let characterWidth = {
  let (_, width) =
    makeLine("a")
    |> BufferLine.getPixelPositionAndWidth(~index=CharacterIndex.zero);
  width;
};
let threeCharacterWidth = 3. *. characterWidth;

describe("WordWrap", ({describe, _}) => {
  describe("none", ({test, _}) => {
    test("no wrap gets full pixel length of line", ({expect, _}) => {
      let line = "abcdef" |> makeLine;
      let wrap = WordWrap.none(line);
      expect.equal(
        wrap,
        (
          [|{byte: ByteIndex.zero, character: CharacterIndex.zero}|],
          6. *. characterWidth,
        ),
      );
    })
  });
  describe("fixed", ({test, _}) => {
    test("#3372 - handle tabs correctly", ({expect, _}) => {
      let line = "\tabc" |> makeLineWithWideTabs;
      let wrap = WordWrap.fixed(~pixels=threeCharacterWidth, line);
      // This should end up with:
      // [
      //  "\ta",
      //  "bc"
      // ]

      expect.equal(
        wrap,
        (
          [|
            {byte: ByteIndex.zero, character: CharacterIndex.zero},
            {
              byte: ByteIndex.(zero + 2),
              character: CharacterIndex.(zero + 2),
            },
          |],
          threeCharacterWidth,
        ),
      );
    });
    test("ascii line within wrap point", ({expect, _}) => {
      let line = "abc" |> makeLine;
      let wrap = WordWrap.fixed(~pixels=threeCharacterWidth, line);
      expect.equal(
        wrap,
        (
          [|{byte: ByteIndex.zero, character: CharacterIndex.zero}|],
          threeCharacterWidth,
        ),
      );
    });
    test("ascii line exceeds wrap point", ({expect, _}) => {
      let line = "abcdef" |> makeLine;
      let wrap = WordWrap.fixed(~pixels=threeCharacterWidth, line);
      expect.equal(
        wrap,
        (
          [|
            {byte: ByteIndex.zero, character: CharacterIndex.zero},
            {
              byte: ByteIndex.(zero + 3),
              character: CharacterIndex.(zero + 3),
            },
          |],
          threeCharacterWidth,
        ),
      );
    });
  });
});
