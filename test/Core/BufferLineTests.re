open EditorCoreTypes;
open Oni_Core;

open TestFramework;

let makeLine = BufferLine.make(~indentation=IndentationSettings.default);

let character = idx => CharacterIndex.ofInt(idx);

describe("BufferLine", ({describe, _}) => {
  let getByte = (line, byteIdx) => {
    let byte = ByteIndex.ofInt(byteIdx);
    BufferLine.getIndex(~byte, line) |> CharacterIndex.toInt;
  };

  describe("getIndexExn", ({test, _}) => {
    test("simple ASCII string", ({expect, _}) => {
      let line = "abc" |> makeLine;

      let getByte = getByte(line);

      expect.equal(getByte(0), 0);
      expect.equal(getByte(1), 1);
      expect.equal(getByte(2), 2);
    });
    test("UTF-8 text: κόσμε", ({expect, _}) => {
      let line = "κόσμε" |> makeLine;

      let getByte = getByte(line);

      expect.equal(getByte(0), 0);
      expect.equal(getByte(1), 0);

      expect.equal(getByte(2), 1);
      expect.equal(getByte(3), 1);
      expect.equal(getByte(4), 1);

      expect.equal(getByte(5), 2);
      expect.equal(getByte(6), 2);

      expect.equal(getByte(7), 3);
      expect.equal(getByte(8), 3);

      expect.equal(getByte(9), 4);
      expect.equal(getByte(10), 4);
    });

    test("UTF-8 text (cached): κόσμε", ({expect, _}) => {
      let line = "κόσμε" |> makeLine;

      let getByte = getByte(line);

      expect.equal(getByte(10), 4);
      expect.equal(getByte(9), 4);
      expect.equal(getByte(0), 0);
      expect.equal(getByte(1), 0);

      expect.equal(getByte(2), 1);
      expect.equal(getByte(3), 1);
      expect.equal(getByte(4), 1);

      expect.equal(getByte(5), 2);
      expect.equal(getByte(6), 2);

      expect.equal(getByte(7), 3);
      expect.equal(getByte(8), 3);
    });
  });
  describe("subExn", ({test, _}) => {
    test("sub in middle of string", ({expect, _}) => {
      let bufferLine = makeLine("abcd");

      let str =
        BufferLine.subExn(~index=character(1), ~length=2, bufferLine);
      expect.string(str).toEqual("bc");
    });
    test("clamps to end of string", ({expect, _}) => {
      let bufferLine = makeLine("abcd");

      let str =
        BufferLine.subExn(~index=character(1), ~length=10, bufferLine);
      expect.string(str).toEqual("bcd");
    });
  });
  describe("lengthBounded", ({test, _}) => {
    test("max less than total length", ({expect, _}) => {
      let bufferLine = makeLine("abc");

      let len = BufferLine.lengthBounded(~max=character(2), bufferLine);
      expect.int(len).toBe(2);
    });
    test("max greater than total length", ({expect, _}) => {
      let bufferLine = makeLine("abc");

      let len = BufferLine.lengthBounded(~max=character(5), bufferLine);
      expect.int(len).toBe(3);
    });
  });
  describe("getByteFromIndex", ({test, _}) => {
    test("clamps to byte 0", ({expect, _}) => {
      let bufferLine = makeLine("abc");
      let byte =
        bufferLine |> BufferLine.getByteFromIndex(~index=character(-1));
      expect.int(byte |> ByteIndex.toInt).toBe(0);
    })
  });
  describe("getPixelPositionAndWidth", ({test, _}) => {
    test("UTF-8: Hiragana あ", ({expect, _}) => {
      let str = "あa";
      let bufferLine = makeLine(str);

      let (position, width) =
        BufferLine.getPixelPositionAndWidth(~index=character(0), bufferLine);

      expect.float(position).toBeCloseTo(0.);
      expect.float(width).toBeCloseTo(14.);

      let (position, width) =
        BufferLine.getPixelPositionAndWidth(~index=character(1), bufferLine);

      expect.float(position).toBeCloseTo(14.);
      expect.float(width).toBeCloseTo(22.4 -. 14.);
    });
    test("negative index should not throw", ({expect, _}) => {
      let bufferLine = makeLine("abc");
      let (position, width) =
        BufferLine.getPixelPositionAndWidth(
          ~index=character(-1),
          bufferLine,
        );

      expect.float(position).toBeCloseTo(0.);
      expect.float(width).toBeCloseTo(8.4);
    });
    test("empty line", ({expect, _}) => {
      let bufferLine = makeLine("");
      let (position, width) =
        BufferLine.getPixelPositionAndWidth(~index=character(0), bufferLine);

      expect.float(position).toBeCloseTo(0.);
      expect.float(width).toBeCloseTo(8.4);
    });
    test("position past end of string", ({expect, _}) => {
      let bufferLine = makeLine("abc");
      let (position, width) =
        BufferLine.getPixelPositionAndWidth(~index=character(4), bufferLine);

      expect.float(position).toBeCloseTo(25.2);
      expect.float(width).toBeCloseTo(8.4);
    });
    test("tab settings are respected for width", ({expect, _}) => {
      let indentation =
        IndentationSettings.create(~mode=Tabs, ~size=2, ~tabSize=3, ());

      let bufferLine = BufferLine.make(~indentation, "\t");
      let (_position, width) =
        BufferLine.getPixelPositionAndWidth(~index=character(0), bufferLine);
      expect.float(width).toBeCloseTo(3. *. 8.4);
    });
    test("tab settings impact position", ({expect, _}) => {
      let indentation =
        IndentationSettings.create(~mode=Tabs, ~size=2, ~tabSize=3, ());

      let bufferLine = BufferLine.make(~indentation, "\ta");
      let (position, width) =
        BufferLine.getPixelPositionAndWidth(~index=character(1), bufferLine);
      expect.float(width).toBeCloseTo(8.4);
      expect.float(position).toBeCloseTo(3. *. 8.4);
    });
  });
  describe("getIndexFromPixel", ({test, _}) => {
    test("position mapped to pixel", ({expect, _}) => {
      let indentation =
        IndentationSettings.create(~mode=Tabs, ~size=8, ~tabSize=8, ());

      let bufferLine = BufferLine.make(~indentation, "\ta");
      print_endline(bufferLine |> BufferLine.raw);
      let characterIndex =
        BufferLine.Slow.getIndexFromPixel(~pixel=0., bufferLine);
      expect.int(characterIndex |> CharacterIndex.toInt).toBe(0);

      let characterIndex =
        BufferLine.Slow.getIndexFromPixel(~pixel=7., bufferLine);
      expect.int(characterIndex |> CharacterIndex.toInt).toBe(0);

      let characterIndex =
        BufferLine.Slow.getIndexFromPixel(~pixel=8. *. 8.4, bufferLine);
      expect.int(characterIndex |> CharacterIndex.toInt).toBe(1);

      let characterIndex =
        BufferLine.Slow.getIndexFromPixel(~pixel=100., bufferLine);
      expect.int(characterIndex |> CharacterIndex.toInt).toBe(1);
    })
  });
  describe("traverse", ({test, _}) => {
    let alwaysTrue = _ => true;
    let onlyA = char => Uchar.to_char(char) == 'A';
    let characterIndex = CharacterIndex.ofInt;
    let traverse = BufferLine.traverse;
    test("empty bufferline", ({expect, _}) => {
      expect.equal(
        makeLine("")
        |> traverse(
             ~f=alwaysTrue,
             ~direction=`Forwards,
             ~index=CharacterIndex.zero,
           ),
        characterIndex(0),
      )
    });
    test("out-of-bounds is pass-through", ({expect, _}) => {
      expect.equal(
        makeLine("")
        |> traverse(
             ~f=alwaysTrue,
             ~direction=`Forwards,
             ~index=characterIndex(99),
           ),
        characterIndex(99),
      )
    });
    test("travel forwards to end", ({expect, _}) => {
      expect.equal(
        makeLine("AAA")
        |> traverse(
             ~f=alwaysTrue,
             ~direction=`Forwards,
             ~index=characterIndex(1),
           ),
        characterIndex(2),
      )
    });
    test("travel backwards to beginning", ({expect, _}) => {
      expect.equal(
        makeLine("AAA")
        |> traverse(
             ~f=alwaysTrue,
             ~direction=`Backwards,
             ~index=characterIndex(1),
           ),
        characterIndex(0),
      )
    });
    test("travel backwards to beginning", ({expect, _}) => {
      expect.equal(
        makeLine("AAA")
        |> traverse(
             ~f=alwaysTrue,
             ~direction=`Backwards,
             ~index=characterIndex(1),
           ),
        characterIndex(0),
      )
    });
    test("travel backwards to word boundary", ({expect, _}) => {
      expect.equal(
        makeLine("bAAAb")
        |> traverse(
             ~f=onlyA,
             ~direction=`Backwards,
             ~index=characterIndex(2),
           ),
        characterIndex(1),
      )
    });
    test("travel forwards to word boundary", ({expect, _}) => {
      expect.equal(
        makeLine("bAAAb")
        |> traverse(~f=onlyA, ~direction=`Forwards, ~index=characterIndex(2)),
        characterIndex(3),
      )
    });
  });
});
