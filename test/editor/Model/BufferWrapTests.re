open Oni_Core.Types;
open Oni_Model;

open TestFramework;

let abcdBuffer = Buffer.ofLines([|"abcd"|]);
let smallBuffer = Buffer.ofLines([|"Hello World"|]);

describe("BufferWrap", ({describe, _}) => {
  describe("getVirtualLines", ({test, _}) => {
    test("basic virtual line validation", ({expect}) => {
      let w = BufferWrap.create(smallBuffer, 6);

      let vlineCount = BufferWrap.getVirtualLineCount(w);
      let line1 = BufferWrap.getVirtualLine(0, smallBuffer, w);
      let line2 = BufferWrap.getVirtualLine(1, smallBuffer, w);

      expect.int(vlineCount).toBe(2);
      expect.string(line1).toEqual("Hello ");
      expect.string(line2).toEqual("World");
    })
  });
  describe("bufferPositionToVirtual", ({test, _}) => {
    test("simple wrapping", ({expect}) => {
      let w = BufferWrap.create(abcdBuffer, 1);
	  let p0 = Position.fromIndices0(0, 0);
	  let p2 = Position.fromIndices0(0, 2);
	  let p3 = Position.fromIndices0(0, 3);

	  let (line0, char0) = BufferWrap.bufferPositionToVirtual(p0, w) |> Position.toIndices0;
	  expect.int(line0).toBe(0);
	  expect.int(char0).toBe(0);

	  let (line2, char2) = BufferWrap.bufferPositionToVirtual(p2, w) |> Position.toIndices0;
	  expect.int(line2).toBe(2);
	  expect.int(char2).toBe(0);

	  let (line3, char3) = BufferWrap.bufferPositionToVirtual(p3, w) |> Position.toIndices0;
	  expect.int(line3).toBe(3);
	  expect.int(char3).toBe(0);
    });
  });
});
