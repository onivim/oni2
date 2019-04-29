open Oni_Model;

open TestFramework;

let smallBuffer = Buffer.ofLines([|"Hello World"|]);

describe("BufferWrap", ({describe, _}) => {
  describe("getVirtualLines", ({test, _}) =>
    test("basic virtual line validation", ({expect}) => {
      let w = BufferWrap.create(smallBuffer, 6);

      let vlineCount = BufferWrap.getVirtualLineCount(w);
      let line1 = BufferWrap.getVirtualLine(0, smallBuffer, w);
      let line2 = BufferWrap.getVirtualLine(1, smallBuffer, w);

      expect.int(vlineCount).toBe(2);
      expect.string(line1).toEqual("Hello ");
      expect.string(line2).toEqual("World");
    })
  );
  describe("bufferPositionToVirtual", ({test, _}) =>
    test("--", ({expect}) =>
      expect.int(0).toBe(2)
    )
  );
  describe("bufferRangeToVirtual", ({test, _}) =>
    test("--", ({expect}) =>
      expect.int(0).toBe(2)
    )
  );
});
