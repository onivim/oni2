/* open Oni_Core; */
open TestFramework;

open Oni_Core.Types;
module LineWrap = Oni_Core.LineWrap;
module WrapMode = Oni_Core.WrapMode;

let line0 = "";
let line2 = "ab";
let line5 = "abcde";

describe("LineWrap", ({describe, _}) => {
  describe("count", ({test, _}) => {
    test("empty line", ({expect}) => {
      let lw = LineWrap.create(line0, WrapMode.column(10));
      expect.int(LineWrap.count(lw)).toBe(1);
    });
    test("line smaller than wrap point", ({expect}) => {
      let lw = LineWrap.create(line2, WrapMode.column(10));
      expect.int(LineWrap.count(lw)).toBe(1);
    });
    test("line at wrap point boundary", ({expect}) => {
      let lw = LineWrap.create(line2, WrapMode.column(2));
      expect.int(LineWrap.count(lw)).toBe(1);
    });
    test("line split multiple times", ({expect}) => {
      let lw = LineWrap.create(line5, WrapMode.column(2));
      expect.int(LineWrap.count(lw)).toBe(3);
    });
  });
  describe("toVirtualPosition", ({test, _}) => {
    test("unwrapped position", ({expect}) => {
      let lw = LineWrap.create(line5, WrapMode.column(3));
      let (line0, column0) =
        LineWrap.toVirtualPosition(Index.ofInt0(1), lw)
        |> Position.toIndices0;

      expect.int(line0).toBe(0);
      expect.int(column0).toBe(1);
    });
    test("wrapped position", ({expect}) => {
      let lw = LineWrap.create(line5, WrapMode.column(3));
      let (line0, column0) =
        LineWrap.toVirtualPosition(Index.ofInt0(3), lw)
        |> Position.toIndices0;

      expect.int(line0).toBe(1);
      expect.int(column0).toBe(0);
    });
  });
});
