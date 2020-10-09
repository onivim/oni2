open EditorCoreTypes;
open Oni_Core;
open Feature_Editor;
open TestFramework;
module LineNumber = EditorCoreTypes.LineNumber;

let makeBuffer = lines =>
  Oni_Core.Buffer.ofLines(~font=Font.default(), lines);

let simpleAsciiBuffer =
  [|"abcdef", "ghijkl"|] |> makeBuffer |> EditorBuffer.ofBuffer;

let bytePosition = (lnum, byteNum) =>
  BytePosition.{
    line: LineNumber.ofZeroBased(lnum),
    byte: ByteIndex.ofInt(byteNum),
  };

let wrapResult = (~line, ~byte, ~character) =>
  Wrapping.{
    line: line |> LineNumber.ofZeroBased,
    byteOffset: byte |> ByteIndex.ofInt,
    characterOffset: character |> CharacterIndex.ofInt,
  };

let (_, aWidth) =
  [|"a"|]
  |> makeBuffer
  |> Oni_Core.Buffer.getLine(0)
  |> BufferLine.getPixelPositionAndWidth(~index=CharacterIndex.zero);

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
        wrapResult(~line=0, ~byte=0, ~character=0),
      );
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=1, wrapping),
        wrapResult(~line=1, ~byte=0, ~character=0),
      );
    });
    test("numberOfLines", ({expect, _}) => {
      expect.int(Wrapping.numberOfLines(wrapping)).toBe(2)
    });
  });

  describe("fixed pixel width (3 characters)", ({test, _}) => {
    let characterWidth = aWidth;
    let threeCharacterWidth = 3. *. characterWidth;
    let wrap = WordWrap.fixed(~pixels=threeCharacterWidth);
    let wrapping = Wrapping.make(~wrap, ~buffer=simpleAsciiBuffer);
    test("update: number of wraps stay the same", ({expect, _}) => {
      let startBuffer = makeBuffer([|"aaa"|]);

      let wrapping =
        Wrapping.make(~wrap, ~buffer=startBuffer |> EditorBuffer.ofBuffer);

      let update =
        BufferUpdate.{
          id: 0,
          startLine: LineNumber.ofZeroBased(0),
          endLine: LineNumber.ofZeroBased(1),
          lines: [|"aaa"|],
          isFull: false,
          version: 999,
        };

      let newBuffer =
        Buffer.update(startBuffer, update) |> EditorBuffer.ofBuffer;

      let wrapping' = Wrapping.update(~update, ~newBuffer, wrapping);

      expect.int(Wrapping.numberOfLines(wrapping')).toBe(1);
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=0, wrapping'),
        wrapResult(~line=0, ~byte=0, ~character=0),
      );
    });

    test("update: increase number of wraps in single line", ({expect, _}) => {
      let startBuffer = makeBuffer([|"aaa"|]);

      let wrapping =
        Wrapping.make(~wrap, ~buffer=startBuffer |> EditorBuffer.ofBuffer);

      let update =
        BufferUpdate.{
          id: 0,
          startLine: LineNumber.ofZeroBased(0),
          endLine: LineNumber.ofZeroBased(1),
          lines: [|"aaaa"|],
          isFull: false,
          version: 999,
        };

      let newBuffer =
        Buffer.update(startBuffer, update) |> EditorBuffer.ofBuffer;

      let wrapping' = Wrapping.update(~update, ~newBuffer, wrapping);

      expect.int(Wrapping.numberOfLines(wrapping')).toBe(2);
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=0, wrapping'),
        wrapResult(~line=0, ~byte=0, ~character=0),
      );
      expect.equal(
        Wrapping.viewLineToBufferPosition(~line=1, wrapping'),
        wrapResult(~line=0, ~byte=3, ~character=3),
      );
    });

    test("bufferLineByteToViewLine", ({expect, _}) => {
      expect.int(
        Wrapping.bufferBytePositionToViewLine(
          ~bytePosition=bytePosition(0, 0),
          wrapping,
        ),
      ).
        toBe(
        0,
      );
      expect.int(
        Wrapping.bufferBytePositionToViewLine(
          ~bytePosition=bytePosition(0, 3),
          wrapping,
        ),
      ).
        toBe(
        1,
      );
      expect.int(
        Wrapping.bufferBytePositionToViewLine(
          ~bytePosition=bytePosition(1, 0),
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
