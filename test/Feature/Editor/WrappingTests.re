open Oni_Core;
open Feature_Editor;
open TestFramework;

let simpleAsciiBuffer =
  [|"abcdef", "ghijkl"|] |> Oni_Core.Buffer.ofLines |> EditorBuffer.ofBuffer;

describe("Wrapping", ({describe, _}) => {
  describe("nowrap", ({test, _}) => {
    let wrap = WordWrap.none;
    let wrapping = Wrapping.make(~wrap, ~buffer=simpleAsciiBuffer);
    test("bufferLineByteToViewLine", ({expect, _}) => {
      expect.int(
        Wrapping.bufferLineByteToViewLine(~line=0, ~byteIndex=0, wrapping),
      ).
        toBe(
        0,
      );
      expect.int(
        Wrapping.bufferLineByteToViewLine(~line=1, ~byteIndex=0, wrapping),
      ).
        toBe(
        1,
      );
    });
    test("viewLineToBufferPosition", ({expect, _}) => {
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=0, wrapping),
        Wrapping.{line: 0, byteOffset: 0, characterOffset: 0},
      );
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=1, wrapping),
        Wrapping.{line: 1, byteOffset: 0, characterOffset: 0},
      );
    });
    test("numberOfLines", ({expect, _}) => {
      expect.int(Wrapping.numberOfLines(wrapping)).toBe(2)
    });
  });
  describe("fixed=3", ({test, _}) => {
    let wrap = WordWrap.fixed(~columns=3);
    let wrapping = Wrapping.make(~wrap, ~buffer=simpleAsciiBuffer);
    test("bufferLineByteToViewLine", ({expect, _}) => {
      expect.int(
        Wrapping.bufferLineByteToViewLine(~line=0, ~byteIndex=0, wrapping),
      ).
        toBe(
        0,
      );
      expect.int(
        Wrapping.bufferLineByteToViewLine(~line=0, ~byteIndex=3, wrapping),
      ).
        toBe(
        1,
      );
      expect.int(
        Wrapping.bufferLineByteToViewLine(~line=1, ~byteIndex=0, wrapping),
      ).
        toBe(
        2,
      );
    });
    test("viewLineToBufferPosition", ({expect, _}) => {
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=0, wrapping),
        Wrapping.{line: 0, byteOffset: 0, characterOffset: 0},
      );
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=1, wrapping),
        Wrapping.{line: 0, byteOffset: 3, characterOffset: 3},
      );
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=2, wrapping),
        Wrapping.{line: 1, byteOffset: 0, characterOffset: 0},
      );
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=3, wrapping),
        Wrapping.{line: 1, byteOffset: 3, characterOffset: 3},
      );
    });
    test("numberOfLines", ({expect, _}) => {
      expect.int(Wrapping.numberOfLines(wrapping)).toBe(4)
    });
  });
});
