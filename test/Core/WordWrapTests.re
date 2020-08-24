open EditorCoreTypes;
open Oni_Core;
open TestFramework;

let makeLine = BufferLine.make(~indentation=IndentationSettings.default);

let characterWidth = {
  let (_, width) =
    makeLine("a")
    |> BufferLine.getPixelPositionAndWidth(~index=CharacterIndex.zero);
  width;
};
let threeCharacterWidth = 3. *. characterWidth;

describe("WordWrap", ({describe, _}) =>
  describe("fixed", ({test, _}) => {
    test("ascii line within wrap point", ({expect, _}) => {
      let line = "abc" |> makeLine;
      let wrap = WordWrap.fixed(~pixels=threeCharacterWidth, line);
      expect.equal(
        wrap,
        [{byte: ByteIndex.zero, character: CharacterIndex.zero}],
      );
    });
    test("ascii line exceeds wrap point", ({expect, _}) => {
      let line = "abcdef" |> makeLine;
      let wrap = WordWrap.fixed(~pixels=threeCharacterWidth, line);
      expect.equal(
        wrap,
        [
          {byte: ByteIndex.zero, character: CharacterIndex.zero},
          {byte: ByteIndex.(zero + 3), character: CharacterIndex.(zero + 3)},
        ],
      );
    });
  })
);
