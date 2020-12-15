open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/lines_100.txt");
let input = s => ignore(Vim.input(s));
let key = s => ignore(Vim.key(s));

describe("Window", ({describe, _}) => {
  describe("get / set metrics", ({test, _}) =>
    test("simple test", ({expect, _}) => {
      let _ = resetBuffer();

      Window.setWidth(50);
      Window.setHeight(51);

      expect.int(Window.getWidth()).toBe(50);
      expect.int(Window.getHeight()).toBe(51);
    })
  );

  describe("setTopLeft", ({test, _}) =>
    test("simple test", ({expect, _}) => {
      let _ = resetBuffer();

      Window.setWidth(3);
      Window.setHeight(3);
      Cursor.set(
        BytePosition.{
          line: LineNumber.ofOneBased(2),
          byte: ByteIndex.ofInt(4),
        },
      );
      Window.setTopLeft(2, 3);

      expect.int(Window.getTopLine()).toBe(2);
      expect.int(Window.getLeftColumn()).toBe(3);
    })
  );

  describe("onSplit", ({test, _}) => {
    test("vsp creates split", ({expect, _}) => {
      let _ = resetBuffer();

      let (_context: Context.t, effects: list(Vim.Effect.t)) =
        command("vsp test.txt");

      expect.equal(effects, [WindowSplit(Split.Vertical({filePath: Some("test.txt")}))])

    });

    test("sp creates split", ({expect, _}) => {
      let _ = resetBuffer();

      let (_context: Context.t, effects: list(Vim.Effect.t)) =
        command("sp test2.txt");
      expect.equal(effects, [WindowSplit(Split.Horizontal({filePath: Some("test2.txt")}))])
    });

    test("<C-w>v creates split, with same buffer", ({expect, _}) => {
      let buf = resetBuffer();

      let filePath = Buffer.getFilename(buf);

      key("<c-w>");
      let (_context: Context.t, effects) = Vim.input("v");
      expect.equal(effects, [WindowSplit(Split.Vertical({filePath: filePath}))])

      let newBuf = Vim.Buffer.getCurrent();

      expect.int(Vim.Buffer.getId(newBuf)).toBe(Vim.Buffer.getId(buf));
    });

    test("<C-w>s creates split, with same buffer", ({expect, _}) => {
      let buf = resetBuffer();
      let filePath = Buffer.getFilename(buf);

      key("<c-w>");
      let (_context, effects) = Vim.input("s");

      expect.equal(effects, [WindowSplit(Split.Horizontal({filePath: filePath}))])

      let newBuf = Vim.Buffer.getCurrent();
      expect.int(Vim.Buffer.getId(newBuf)).toBe(Vim.Buffer.getId(buf));
    });
  });
});
