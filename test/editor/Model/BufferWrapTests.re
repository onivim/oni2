open Oni_Core;
open Oni_Core.Types;
open Oni_Model;

open Oni_Core_Test.Helpers;

open TestFramework;

let abcdBuffer = Buffer.ofLines([|"abcd"|]);
let smallBuffer = Buffer.ofLines([|"Hello World"|]);

let r = (startLine, startCharacter, endLine, endCharacter) => Range.ofInt0(~startLine, ~startCharacter, ~endLine, ~endCharacter, ());

describe("BufferWrap", ({describe, _}) => {
  describe("getVirtualLines", ({test, _}) =>
    test("basic virtual line validation", ({expect}) => {
      let w = BufferWrap.create(smallBuffer, WrapMode.column(6));

      let vlineCount = BufferWrap.getVirtualLineCount(w);
      let line1 = BufferWrap.getVirtualLine(0, smallBuffer, w);
      let line2 = BufferWrap.getVirtualLine(1, smallBuffer, w);

      expect.int(vlineCount).toBe(2);
      expect.string(line1).toEqual("Hello ");
      expect.string(line2).toEqual("World");
    })
  );
  describe("bufferPositionToVirtual", ({test, _}) =>
    test("simple wrapping", ({expect}) => {
      let w = BufferWrap.create(abcdBuffer, WrapMode.column(1));
      let p0 = Position.fromIndices0(0, 0);
      let p2 = Position.fromIndices0(0, 2);
      let p3 = Position.fromIndices0(0, 3);

      let (line0, char0) =
        BufferWrap.bufferPositionToVirtual(p0, w) |> Position.toIndices0;
      expect.int(line0).toBe(0);
      expect.int(char0).toBe(0);

      let (line2, char2) =
        BufferWrap.bufferPositionToVirtual(p2, w) |> Position.toIndices0;
      expect.int(line2).toBe(2);
      expect.int(char2).toBe(0);

      let (line3, char3) =
        BufferWrap.bufferPositionToVirtual(p3, w) |> Position.toIndices0;
      expect.int(line3).toBe(3);
  describe("bufferRangeToVirtual", ({test, _}) => {
    test("single range", ({expect}) => {
      let w = BufferWrap.create(abcdBuffer, WrapMode.column(2));
	  let range = r(0, 0, 0, 1);

	  let actual = BufferWrap.bufferRangeToVirtual(range, w);

	  let expected = [
		r(0, 0, 0, 1)
	  ];

	  validateRanges(expect, actual, expected);
	});
    test("single buffer line, expanded to multiple ranges", ({expect}) => {
      let w = BufferWrap.create(abcdBuffer, WrapMode.column(1));
	  let range = Range.ofInt0(~startLine=0, ~startCharacter=0, ~endLine=0, ~endCharacter=3, ());

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
      expect.int(char3).toBe(0);
    })
  );
});
