open Oni_Core;
open Oni_Core.Types;
open Oni_Model;

open Oni_Core_Test.Helpers;

open TestFramework;

let abcdBuffer = Buffer.ofLines([|"abcd"|]);
let smallBuffer = Buffer.ofLines([|"Hello World"|]);
let multiLineBuffer =
  Buffer.ofLines([|"line10line11", "line20line21", "line30line31"|]);
let multiLineBufferId =
  Buffer.getMetadata(multiLineBuffer) |> ((m: BufferMetadata.t) => m.id);

let r = (startLine, startCharacter, endLine, endCharacter) =>
  Range.ofInt0(~startLine, ~startCharacter, ~endLine, ~endCharacter, ());

describe("BufferWrap", ({describe, _}) => {
  describe("getVirtualLines", ({test, _}) => {
    test("basic virtual line validation", ({expect}) => {
      let w = BufferWrap.create(smallBuffer, WrapMode.column(6));

      let vlineCount = BufferWrap.getVirtualLineCount(w);
      let line1 = BufferWrap.getVirtualLine(0, smallBuffer, w);
      let line2 = BufferWrap.getVirtualLine(1, smallBuffer, w);

      expect.int(vlineCount).toBe(2);
      expect.string(line1).toEqual("Hello ");
      expect.string(line2).toEqual("World");
    });

    test("across multiple buffer lines", ({expect}) => {
      let w = BufferWrap.create(multiLineBuffer, WrapMode.column(6));

      let vlineCount = BufferWrap.getVirtualLineCount(w);
      let line10 = BufferWrap.getVirtualLine(0, multiLineBuffer, w);
      let line11 = BufferWrap.getVirtualLine(1, multiLineBuffer, w);
      let line30 = BufferWrap.getVirtualLine(4, multiLineBuffer, w);
      let line31 = BufferWrap.getVirtualLine(5, multiLineBuffer, w);

      expect.int(vlineCount).toBe(6);
      expect.string(line10).toEqual("line10");
      expect.string(line11).toEqual("line11");
      expect.string(line30).toEqual("line30");
      expect.string(line31).toEqual("line31");
    });
  });
  describe("bufferPositionToVirtual", ({test, _}) =>
    test("simple wrapping", ({expect}) => {
      let w = BufferWrap.create(abcdBuffer, WrapMode.column(1));
      let p0 = Position.ofInt0(0, 0);
      let p2 = Position.ofInt0(0, 2);
      let p3 = Position.ofInt0(0, 3);

      let (line0, char0) =
        BufferWrap.bufferPositionToVirtual(p0, w) |> Position.toInt0;
      expect.int(line0).toBe(0);
      expect.int(char0).toBe(0);

      let (line2, char2) =
        BufferWrap.bufferPositionToVirtual(p2, w) |> Position.toInt0;
      expect.int(line2).toBe(2);
      expect.int(char2).toBe(0);

      let (line3, char3) =
        BufferWrap.bufferPositionToVirtual(p3, w) |> Position.toInt0;
      expect.int(line3).toBe(3);
      expect.int(char3).toBe(0);
    })
  );
  describe("bufferRangeToVirtual", ({test, _}) => {
    test("single range", ({expect}) => {
      let w = BufferWrap.create(abcdBuffer, WrapMode.column(2));
      let range = r(0, 0, 0, 1);

      let actual = BufferWrap.bufferRangeToVirtual(range, w);

      let expected = [r(0, 0, 0, 1)];

      validateRanges(expect, actual, expected);
    });
    test("single buffer line, expanded to multiple ranges", ({expect}) => {
      let w = BufferWrap.create(abcdBuffer, WrapMode.column(1));
      let range =
        Range.ofInt0(
          ~startLine=0,
          ~startCharacter=0,
          ~endLine=0,
          ~endCharacter=3,
          (),
        );

      let actual = BufferWrap.bufferRangeToVirtual(range, w);

      let expected = [
        r(0, 0, 0, 1),
        r(1, 0, 1, 1),
        r(2, 0, 2, 1),
        r(3, 0, 3, 1),
      ];

      validateRanges(expect, actual, expected);
    });
  });

  describe("update", ({test, _}) => {
    test("same buffer lines, add virtual lines", ({expect}) => {
      let w = BufferWrap.create(multiLineBuffer, WrapMode.column(6));

      let idx = Index.ofInt0(1);
      let update =
        BufferUpdate.create(
          ~id=multiLineBufferId,
          ~startLine=idx,
          ~endLine=idx,
          ~lines=["line20line21line22"],
          ~version=100,
          (),
        );
      let newBuffer = Buffer.update(multiLineBuffer, update);
      let w = BufferWrap.update(update, w);

      let vlineCount = BufferWrap.getVirtualLineCount(w);
      let line10 = BufferWrap.getVirtualLine(0, newBuffer, w);
      let line11 = BufferWrap.getVirtualLine(1, newBuffer, w);
      let line20 = BufferWrap.getVirtualLine(2, newBuffer, w);
      let line21 = BufferWrap.getVirtualLine(3, newBuffer, w);
      let line22 = BufferWrap.getVirtualLine(4, newBuffer, w);
      let line30 = BufferWrap.getVirtualLine(5, newBuffer, w);
      let line31 = BufferWrap.getVirtualLine(6, newBuffer, w);

      expect.int(vlineCount).toBe(7);
      expect.string(line10).toEqual("line10");
      expect.string(line11).toEqual("line11");
      expect.string(line20).toEqual("line20");
      expect.string(line21).toEqual("line21");
      expect.string(line22).toEqual("line22");
      expect.string(line30).toEqual("line30");
      expect.string(line31).toEqual("line31");
    });

    test("same buffer lines, remove virtual lines", ({expect}) => {
      let w = BufferWrap.create(multiLineBuffer, WrapMode.column(6));

      let idx = Index.ofInt0(1);
      let update =
        BufferUpdate.create(
          ~id=multiLineBufferId,
          ~startLine=idx,
          ~endLine=idx,
          ~lines=["line20"],
          ~version=100,
          (),
        );
      let newBuffer = Buffer.update(multiLineBuffer, update);
      let w = BufferWrap.update(update, w);

      let vlineCount = BufferWrap.getVirtualLineCount(w);
      let line10 = BufferWrap.getVirtualLine(0, newBuffer, w);
      let line11 = BufferWrap.getVirtualLine(1, newBuffer, w);
      let line20 = BufferWrap.getVirtualLine(2, newBuffer, w);
      let line30 = BufferWrap.getVirtualLine(5, newBuffer, w);
      let line31 = BufferWrap.getVirtualLine(6, newBuffer, w);

      expect.int(vlineCount).toBe(5);
      expect.string(line10).toEqual("line10");
      expect.string(line11).toEqual("line11");
      expect.string(line20).toEqual("line20");
      expect.string(line30).toEqual("line30");
      expect.string(line31).toEqual("line31");
    });

    test("add buffer lines", ({expect}) => {
      let w = BufferWrap.create(multiLineBuffer, WrapMode.column(6));

      let idx = Index.ofInt0(1);
      let update =
        BufferUpdate.create(
          ~id=multiLineBufferId,
          ~startLine=idx,
          ~endLine=idx,
          ~lines=["line00", "line10line11"],
          ~version=100,
          (),
        );
      let newBuffer = Buffer.update(multiLineBuffer, update);
      let w = BufferWrap.update(update, w);

      let vlineCount = BufferWrap.getVirtualLineCount(w);
      let line00 = BufferWrap.getVirtualLine(0, newBuffer, w);
      let line10 = BufferWrap.getVirtualLine(1, newBuffer, w);
      let line11 = BufferWrap.getVirtualLine(2, newBuffer, w);
      let line30 = BufferWrap.getVirtualLine(5, newBuffer, w);
      let line31 = BufferWrap.getVirtualLine(6, newBuffer, w);

      expect.int(vlineCount).toBe(7);
      expect.string(line00).toEqual("line00");
      expect.string(line10).toEqual("line10");
      expect.string(line11).toEqual("line11");
      expect.string(line30).toEqual("line30");
      expect.string(line31).toEqual("line31");
    });

    test("remove buffer lines", ({expect}) => {
      let w = BufferWrap.create(multiLineBuffer, WrapMode.column(6));

      let idx = Index.ofInt0(1);
      let update =
        BufferUpdate.create(
          ~id=multiLineBufferId,
          ~startLine=idx,
          ~endLine=idx,
          ~lines=[],
          ~version=100,
          (),
        );
      let newBuffer = Buffer.update(multiLineBuffer, update);
      let w = BufferWrap.update(update, w);

      let vlineCount = BufferWrap.getVirtualLineCount(w);
      let line10 = BufferWrap.getVirtualLine(0, newBuffer, w);
      let line11 = BufferWrap.getVirtualLine(1, newBuffer, w);
      let line30 = BufferWrap.getVirtualLine(2, newBuffer, w);
      let line31 = BufferWrap.getVirtualLine(3, newBuffer, w);

      expect.int(vlineCount).toBe(4);
      expect.string(line10).toEqual("line10");
      expect.string(line11).toEqual("line11");
      expect.string(line30).toEqual("line30");
      expect.string(line31).toEqual("line31");
    });
  });
});
