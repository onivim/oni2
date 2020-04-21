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
  describe("getPositionAndWidth", ({test, _}) => {
    test("UTF-8: Hiragana あ", ({expect, _}) => {
      let str = "あa";
      let bufferLine = makeLine(str);

      let (position, width) =
        BufferLine.getPositionAndWidth(~index=0, bufferLine);

      expect.int(position).toBe(0);
      expect.int(width).toBe(2);

      let (position, width) =
        BufferLine.getPositionAndWidth(~index=1, bufferLine);

      expect.int(position).toBe(2);
      expect.int(width).toBe(1);
    });
    test("empty line", ({expect, _}) => {
      let bufferLine = makeLine("");
      let (position, width) =
        BufferLine.getPositionAndWidth(~index=0, bufferLine);

      expect.int(position).toBe(0);
      expect.int(width).toBe(1);
    });
    test("position past end of string", ({expect, _}) => {
      let bufferLine = makeLine("abc");
      let (position, width) =
        BufferLine.getPositionAndWidth(~index=4, bufferLine);

      expect.int(position).toBe(3);
      expect.int(width).toBe(1);
    });
    test("tab settings are respected for width", ({expect, _}) => {
      let indentation =
        IndentationSettings.create(~mode=Tabs, ~size=2, ~tabSize=3, ());

      let bufferLine = BufferLine.make(~indentation, "\t");
      let (_position, width) =
        BufferLine.getPositionAndWidth(~index=0, bufferLine);
      expect.int(width).toBe(3);
    });
    test("tab settings impact position", ({expect, _}) => {
      let indentation =
        IndentationSettings.create(~mode=Tabs, ~size=2, ~tabSize=3, ());

      let bufferLine = BufferLine.make(~indentation, "\ta");
      let (position, width) =
        BufferLine.getPositionAndWidth(~index=1, bufferLine);
      expect.int(width).toBe(1);
      expect.int(position).toBe(3);
    });
  });
});
