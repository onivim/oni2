open EditorCoreTypes;
open Oni_Core;
open Feature_Editor;
open TestFramework;
module LineNumber = EditorCoreTypes.LineNumber;

let simpleAsciiBuffer =
  [|"abcdef", "ghijkl"|]
  |> Oni_Core.Buffer.ofLines(~font=Font.default())
  |> EditorBuffer.ofBuffer;

let (_, aWidth) =
  [|"a"|]
  |> Oni_Core.Buffer.ofLines(~font=Font.default())
  |> Oni_Core.Buffer.getLine(0)
  |> BufferLine.getPixelPositionAndWidth(~index=CharacterIndex.zero);

prerr_endline("'a' width: " ++ string_of_float(aWidth));

describe("Wrapping", ({describe, _}) => {
  describe("nowrap", ({test, _}) => {
    let wrap = WordWrap.none;
    let wrapping = Wrapping.make(~wrap, ~buffer=simpleAsciiBuffer);
    test("bufferLineByteToViewLine", ({expect, _}) => {
      expect.int(
        Wrapping.bufferBytePositionToViewLine(
          ~bytePosition=
            BytePosition.{line: LineNumber.zero, byte: ByteIndex.zero},
          wrapping,
        ),
      ).
        toBe(
        0,
      );
      expect.int(
        Wrapping.bufferBytePositionToViewLine(
          ~bytePosition=
            BytePosition.{line: LineNumber.(zero + 1), byte: ByteIndex.zero},
          wrapping,
        ),
      ).
        toBe(
        1,
      );
    });
    test("viewLineToBufferPosition", ({expect, _}) => {
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=0, wrapping),
        Wrapping.{
          line: 0 |> LineNumber.ofZeroBased,
          byteOffset: 0 |> ByteIndex.ofInt,
          characterOffset: 0 |> CharacterIndex.ofInt,
        },
      );
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=1, wrapping),
        Wrapping.{
          line: 1 |> LineNumber.ofZeroBased,
          byteOffset: 0 |> ByteIndex.ofInt,
          characterOffset: 0 |> CharacterIndex.ofInt,
        },
      );
    });
    test("numberOfLines", ({expect, _}) => {
      expect.int(Wrapping.numberOfLines(wrapping)).toBe(2)
    });
  });
  describe("fixed=3", ({test, _}) => {
    let characterWidth = aWidth;
    let threeCharacterWidth = 3. *. characterWidth;
    let wrap = WordWrap.fixed(~pixels=threeCharacterWidth);
    let wrapping = Wrapping.make(~wrap, ~buffer=simpleAsciiBuffer);
    test("bufferLineByteToViewLine", ({expect, _}) => {
      expect.int(
        Wrapping.bufferBytePositionToViewLine(
          ~bytePosition=
            BytePosition.{line: LineNumber.zero, byte: ByteIndex.zero},
          wrapping,
        ),
      ).
        toBe(
        0,
      );
      expect.int(
        Wrapping.bufferBytePositionToViewLine(
          ~bytePosition=
            BytePosition.{line: LineNumber.zero, byte: ByteIndex.(zero + 3)},
          wrapping,
        ),
      ).
        toBe(
        1,
      );
      expect.int(
        Wrapping.bufferBytePositionToViewLine(
          ~bytePosition=
            BytePosition.{
              line: LineNumber.(zero + 1),
              byte: ByteIndex.(zero),
            },
          wrapping,
        ),
      ).
        toBe(
        2,
      );
    });
    test("viewLineToBufferPosition", ({expect, _}) => {
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=0, wrapping),
        Wrapping.{
          line: 0 |> LineNumber.ofZeroBased,
          byteOffset: 0 |> ByteIndex.ofInt,
          characterOffset: 0 |> CharacterIndex.ofInt,
        },
      );
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=1, wrapping),
        Wrapping.{
          line: 0 |> LineNumber.ofZeroBased,
          byteOffset: 3 |> ByteIndex.ofInt,
          characterOffset: 3 |> CharacterIndex.ofInt,
        },
      );
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=2, wrapping),
        Wrapping.{
          line: 1 |> LineNumber.ofZeroBased,
          byteOffset: 0 |> ByteIndex.ofInt,
          characterOffset: 0 |> CharacterIndex.ofInt,
        },
      );
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=3, wrapping),
        Wrapping.{
          line: 1 |> LineNumber.ofZeroBased,
          byteOffset: 3 |> ByteIndex.ofInt,
          characterOffset: 3 |> CharacterIndex.ofInt,
        },
      );
    });
    test("numberOfLines", ({expect, _}) => {
      expect.int(Wrapping.numberOfLines(wrapping)).toBe(4)
    });
  });
});
