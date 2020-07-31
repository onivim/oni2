open Oni_Core;

open TestFramework;

let makeLine = BufferLine.make(~indentation=IndentationSettings.default);

describe("BufferLine", ({describe, _}) => {
  describe("getIndexExn", ({test, _}) => {
    test("simple ASCII string", ({expect, _}) => {
      let line = "abc" |> makeLine;

      let getByte = byte => BufferLine.getIndex(~byte, line);

      expect.equal(getByte(0), 0);
      expect.equal(getByte(1), 1);
      expect.equal(getByte(2), 2);
    });
    test("UTF-8 text: κόσμε", ({expect, _}) => {
      let line = "κόσμε" |> makeLine;

      let getByte = byte => BufferLine.getIndex(~byte, line);

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

      let getByte = byte => BufferLine.getIndex(~byte, line);

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

      let str = BufferLine.subExn(~index=1, ~length=2, bufferLine);
      expect.string(str).toEqual("bc");
    });
    test("clamps to end of string", ({expect, _}) => {
      let bufferLine = makeLine("abcd");

      let str = BufferLine.subExn(~index=1, ~length=10, bufferLine);
      expect.string(str).toEqual("bcd");
    });
  });
  describe("lengthBounded", ({test, _}) => {
    test("max less than total length", ({expect, _}) => {
      let bufferLine = makeLine("abc");

      let len = BufferLine.lengthBounded(~max=2, bufferLine);
      expect.int(len).toBe(2);
    });
    test("max greater than total length", ({expect, _}) => {
      let bufferLine = makeLine("abc");

      let len = BufferLine.lengthBounded(~max=5, bufferLine);
      expect.int(len).toBe(3);
    });
  });
  describe("getByteFromIndex", ({test, _}) => {
    test("clamps to byte 0", ({expect, _}) => {
      let bufferLine = makeLine("abc");
      let byte = bufferLine |> BufferLine.getByteFromIndex(~index=-1);
      expect.int(byte).toBe(0);
    })
  });
  describe("getPixelPositionAndWidth", ({test, _}) => {
    test("UTF-8: Hiragana あ", ({expect, _}) => {
      let str = "あa";
      let bufferLine = makeLine(str);

      let (position, width) =
        BufferLine.getPixelPositionAndWidth(~index=0, bufferLine);

      expect.float(position).toBeCloseTo(0.);
      expect.float(width).toBeCloseTo(14.);

      let (position, width) =
        BufferLine.getPixelPositionAndWidth(~index=1, bufferLine);

      expect.float(position).toBeCloseTo(14.);
      expect.float(width).toBeCloseTo(22.4 -. 14.);
    });
    test("negative index should not throw", ({expect, _}) => {
      let bufferLine = makeLine("abc");
      let (position, width) =
        BufferLine.getPixelPositionAndWidth(~index=-1, bufferLine);

      expect.float(position).toBeCloseTo(0.);
      expect.float(width).toBeCloseTo(8.4);
    });
    test("empty line", ({expect, _}) => {
      let bufferLine = makeLine("");
      let (position, width) =
        BufferLine.getPixelPositionAndWidth(~index=0, bufferLine);

      expect.float(position).toBeCloseTo(0.);
      expect.float(width).toBeCloseTo(8.4);
    });
    test("position past end of string", ({expect, _}) => {
      let bufferLine = makeLine("abc");
      let (position, width) =
        BufferLine.getPixelPositionAndWidth(~index=4, bufferLine);

      expect.float(position).toBeCloseTo(25.2);
      expect.float(width).toBeCloseTo(8.4);
    });
    test("tab settings are respected for width", ({expect, _}) => {
      let indentation =
        IndentationSettings.create(~mode=Tabs, ~size=2, ~tabSize=3, ());

      let bufferLine = BufferLine.make(~indentation, "\t");
      let (_position, width) =
        BufferLine.getPixelPositionAndWidth(~index=0, bufferLine);
      expect.float(width).toBeCloseTo(3. *. 8.4);
    });
    test("tab settings impact position", ({expect, _}) => {
      let indentation =
        IndentationSettings.create(~mode=Tabs, ~size=2, ~tabSize=3, ());

      let bufferLine = BufferLine.make(~indentation, "\ta");
      let (position, width) =
        BufferLine.getPixelPositionAndWidth(~index=1, bufferLine);
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
      let byteIndex =
        BufferLine.Slow.getIndexFromPixel(~pixel=0., bufferLine);
      expect.int(byteIndex).toBe(0);

      let byteIndex =
        BufferLine.Slow.getIndexFromPixel(~pixel=7., bufferLine);
      expect.int(byteIndex).toBe(0);

      let byteIndex =
        BufferLine.Slow.getIndexFromPixel(~pixel=8. *. 8.4, bufferLine);
      expect.int(byteIndex).toBe(1);

      let byteIndex =
        BufferLine.Slow.getIndexFromPixel(~pixel=100., bufferLine);
      expect.int(byteIndex).toBe(1);
    })
  });
});
