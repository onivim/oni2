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

      let splits = ref([]);
      let dispose =
        Window.onSplit((splitType, name) =>
          splits := [(splitType, name), ...splits^]
        );

      let (_context: Context.t, _effects: list(Vim.Effect.t)) =
        command("vsp test.txt");

      expect.int(List.length(splits^)).toBe(1);

      let (splitType, name) = List.hd(splits^);

      expect.bool(splitType == Types.Vertical).toBe(true);
      expect.string(name).toEqual("test.txt");

      dispose();
    });

    test("sp creates split", ({expect, _}) => {
      let _ = resetBuffer();

      let splits = ref([]);
      let dispose =
        Window.onSplit((splitType, name) =>
          splits := [(splitType, name), ...splits^]
        );

      let (_context: Context.t, _effects: list(Vim.Effect.t)) =
        command("sp test2.txt");

      expect.int(List.length(splits^)).toBe(1);

      let (splitType, name) = List.hd(splits^);

      expect.bool(splitType == Types.Horizontal).toBe(true);
      expect.string(name).toEqual("test2.txt");

      dispose();
    });

    test("<C-w>v creates split, with same buffer", ({expect, _}) => {
      let buf = resetBuffer();

      let splits = ref([]);
      let dispose =
        Window.onSplit((splitType, name) =>
          splits := [(splitType, name), ...splits^]
        );

      key("<c-w>");
      input("v");

      expect.int(List.length(splits^)).toBe(1);

      let (splitType, _) = List.hd(splits^);

      let newBuf = Vim.Buffer.getCurrent();

      expect.bool(splitType == Types.Vertical).toBe(true);
      expect.int(Vim.Buffer.getId(newBuf)).toBe(Vim.Buffer.getId(buf));

      dispose();
    });

    test("<C-w>s creates split, with same buffer", ({expect, _}) => {
      let buf = resetBuffer();

      let splits = ref([]);
      let dispose =
        Window.onSplit((splitType, name) =>
          splits := [(splitType, name), ...splits^]
        );

      key("<c-w>");
      input("s");

      expect.int(List.length(splits^)).toBe(1);

      let (splitType, _) = List.hd(splits^);

      let newBuf = Vim.Buffer.getCurrent();

      expect.bool(splitType == Types.Horizontal).toBe(true);
      expect.int(Vim.Buffer.getId(newBuf)).toBe(Vim.Buffer.getId(buf));

      dispose();
    });
  });
});
