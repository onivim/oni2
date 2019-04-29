open Oni_Model;

open TestFramework;

let smallBuffer = Buffer.ofLines([|"Hello World"|]);

describe("BufferWrap", ({describe, _}) =>
  describe("getVirtualLines", ({test, _}) =>
    test("basic virtual line validation", ({expect}) => {
      let w = BufferWrap.create(smallBuffer, 8);

      let vlineCount = BufferWrap.getVirtualLineCount(w);
      expect.int(vlineCount).toBe(2);
    })
  )
);
