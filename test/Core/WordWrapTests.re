open EditorCoreTypes;
open Oni_Core;
open TestFramework;

let makeLine = BufferLine.make(~indentation=IndentationSettings.default);

describe("WordWrap", ({describe, _}) =>
  describe("fixed", ({test, _}) => {
    test("ascii line within wrap point", ({expect, _}) => {
      let line = "abc" |> makeLine;
      let wrap = WordWrap.fixed(~pixels=3., line);
      expect.equal(
        wrap,
        [{byte: ByteIndex.zero, character: CharacterIndex.zero}],
      );
    });
    test("ascii line exceeds wrap point", ({expect, _}) => {
      let line = "abcdef" |> makeLine;
      let wrap = WordWrap.fixed(~pixels=3., line);
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
