//open EditorCoreTypes;
open Oni_Core;

open TestFramework;
//open Helpers;

let makeLine = BufferLine.make(~indentation=IndentationSettings.default);

describe("BufferLine", ({describe, _}) => {
  describe("unsafeSub", ({test, _}) => {
    test("sub in middle of string", ({expect}) => {
      let bufferLine = makeLine("abcd"); 

      let str = BufferLine.unsafeSub(~index=1, ~length=2, bufferLine);
      expect.string(str).toEqual("bc");
    });
    test("clamps to end of string", ({expect}) => {
      let bufferLine = makeLine("abcd"); 

      let str = BufferLine.unsafeSub(~index=1, ~length=10, bufferLine);
      expect.string(str).toEqual("bcd");
    });
  });
  describe("boundedLengthUtf8", ({test, _}) => {
    test("max less than total length", ({expect}) => {
      let bufferLine = makeLine("abc"); 

      let len = BufferLine.boundedLengthUtf8(~max=2, bufferLine);
      expect.int(len).toBe(2);
    });
    test("max greater than total length", ({expect}) => {
      let bufferLine = makeLine("abc"); 

      let len = BufferLine.boundedLengthUtf8(~max=5, bufferLine);
      expect.int(len).toBe(3);
    });
  });
  describe("getPositionAndWidth", ({test, _}) => {
    test("empty line", ({expect}) => {
      let bufferLine = makeLine("");
      let (position, width) = BufferLine.getPositionAndWidth(~index=0, bufferLine);

      expect.int(position).toBe(0);
      expect.int(width).toBe(1);
    });
    test("position past end of string", ({expect}) => {
       
      let bufferLine = makeLine("abc");
      let (position, width) = BufferLine.getPositionAndWidth(~index=4, bufferLine);

      expect.int(position).toBe(3);
      expect.int(width).toBe(1);
    });
    test("tab settings are respected for width", ({expect}) => {
      let indentation = IndentationSettings.create(~mode=Tabs, ~size=2, ~tabSize=3, ());

      let bufferLine = BufferLine.make(~indentation, "\t");
      let (_position, width) = BufferLine.getPositionAndWidth(~index=0, bufferLine);
      expect.int(width).toBe(3);
    });
    test("tab settings impact position", ({expect}) => {
      let indentation = IndentationSettings.create(~mode=Tabs, ~size=2, ~tabSize=3, ());

      let bufferLine = BufferLine.make(~indentation, "\ta");
      let (position, width) = BufferLine.getPositionAndWidth(~index=1, bufferLine);
      expect.int(width).toBe(1);
      expect.int(position).toBe(3);
    });
  })
});
