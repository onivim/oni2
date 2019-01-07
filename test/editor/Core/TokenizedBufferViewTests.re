open Oni_Core;
/* open Oni_Core.Types; */
open TestFramework;

describe("TokenizedBufferView", ({describe, _}) =>
  describe("ofTokenizedBuffer", ({test, _}) =>
    test("simple buffer without wrapping", ({expect, _}) => {
      let view =
        Buffer.ofLines([|"line1", "line2"|])
        |> TokenizedBuffer.ofBuffer
        |> TokenizedBufferView.ofTokenizedBuffer;

      expect.int(Array.length(view.viewLines)).toBe(2);
    })
  )
);
