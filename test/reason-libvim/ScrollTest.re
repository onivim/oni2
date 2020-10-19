open TestFramework;

open Vim;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");

describe("Scroll", ({describe, _}) => {
  describe("horizontal", ({test, _}) => {
    test("zh", ({expect, _}) => {
      let _ = resetBuffer();

      let (_context: Context.t, effects: list(Effect.t)) = input("zh");

      expect.list(effects).toContainEqual(
        Effect.Scroll({count: 1, direction: Scroll.ColumnRight}),
      );
    })
  });
  describe("vertical", ({test, _}) => {
    test("zz", ({expect, _}) => {
      let _ = resetBuffer();

      let (_context: Context.t, effects: list(Effect.t)) = input("zz");

      expect.list(effects).toContainEqual(
        Effect.Scroll({count: 1, direction: Scroll.CursorCenterVertically}),
      );
    })
  });
});
