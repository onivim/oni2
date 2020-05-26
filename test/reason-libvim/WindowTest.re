open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/lines_100.txt");
let input = s => ignore(Vim.input(s));

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
      Cursor.setLocation(
        ~line=Index.fromOneBased(2),
        ~column=Index.fromZeroBased(4),
      );
      Window.setTopLeft(2, 3);

      expect.int(Window.getTopLine()).toBe(2);
      expect.int(Window.getLeftColumn()).toBe(3);
    })
  );

  describe("onLeftColumnChanged", ({test, _}) =>
    test("dispatches on change", ({expect, _}) => {
      let _ = resetBuffer();

      Window.setWidth(3);
      Window.setHeight(3);

      let leftColumnChanges: ref(list(int)) = ref([]);
      let dispose =
        Window.onLeftColumnChanged(tl =>
          leftColumnChanges := [tl, ...leftColumnChanges^]
        );

      input("$");

      expect.int(List.length(leftColumnChanges^)).toBe(1);
      expect.int(List.hd(leftColumnChanges^)).toBe(4);

      input("0");
      expect.int(List.length(leftColumnChanges^)).toBe(2);
      expect.int(List.hd(leftColumnChanges^)).toBe(0);

      dispose();
    })
  );

  describe("onTopLineChanged", ({test, _}) =>
    test("dispatches on change", ({expect, _}) => {
      let _ = resetBuffer();

      Window.setWidth(50);
      Window.setHeight(20);

      let topLineChanges: ref(list(int)) = ref([]);
      let dispose =
        Window.onTopLineChanged(tl =>
          topLineChanges := [tl, ...topLineChanges^]
        );

      input(":");
      input("5");
      input("0");
      input("<cr>");

      input("z");
      input("z");

      expect.int(List.length(topLineChanges^)).toBe(1);
      expect.int(List.hd(topLineChanges^)).toBe(41);

      input("z");
      input("b");

      expect.int(List.length(topLineChanges^)).toBe(2);
      expect.int(List.hd(topLineChanges^)).toBe(31);

      input("z");
      input("t");

      expect.int(List.length(topLineChanges^)).toBe(3);
      expect.int(List.hd(topLineChanges^)).toBe(50);

      dispose();
    })
  );

  describe("onMovement", ({test, _}) => {
    test("simple movement", ({expect, _}) => {
      let _ = resetBuffer();

      let movements = ref([]);
      let dispose =
        Window.onMovement((movementType, count) =>
          movements := [(movementType, count), ...movements^]
        );

      input("<c-w>");
      input("j");

      expect.int(List.length(movements^)).toBe(1);

      let (movementType, count) = List.hd(movements^);

      expect.bool(movementType == Types.OneDown).toBe(true);
      expect.int(count).toBe(1);

      dispose();
    });

    test("movement with count", ({expect, _}) => {
      let _ = resetBuffer();

      let movements = ref([]);
      let dispose =
        Window.onMovement((movementType, count) =>
          movements := [(movementType, count), ...movements^]
        );

      input("5");
      input("<c-w>");
      input("h");

      expect.int(List.length(movements^)).toBe(1);

      let (movementType, count) = List.hd(movements^);

      expect.bool(movementType == Types.OneLeft).toBe(true);
      expect.int(count).toBe(5);

      dispose();
    });

    test("rotate downwards", ({expect, _}) => {
      let _ = resetBuffer();

      let movements = ref([]);
      let dispose =
        Window.onMovement((movementType, count) =>
          movements := [(movementType, count), ...movements^]
        );

      input("<c-w>");
      input("r");

      expect.int(List.length(movements^)).toBe(1);

      let (movementType, count) = List.hd(movements^);

      expect.bool(movementType == Types.RotateDownwards).toBe(true);
      expect.int(count).toBe(1);

      dispose();
    });

    test("rotate upwards", ({expect, _}) => {
      let _ = resetBuffer();

      let movements = ref([]);
      let dispose =
        Window.onMovement((movementType, count) =>
          movements := [(movementType, count), ...movements^]
        );

      input("<c-w>");
      input("R");

      expect.int(List.length(movements^)).toBe(1);

      let (movementType, count) = List.hd(movements^);

      expect.bool(movementType == Types.RotateUpwards).toBe(true);
      expect.int(count).toBe(1);

      dispose();
    });
  });
  describe("onSplit", ({test, _}) => {
    test("vsp creates split", ({expect, _}) => {
      let _ = resetBuffer();

      let splits = ref([]);
      let dispose =
        Window.onSplit((splitType, name) =>
          splits := [(splitType, name), ...splits^]
        );

      let _context: Context.t = command("vsp test.txt");

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

      let _context: Context.t = command("sp test2.txt");

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

      input("<c-w>");
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

      input("<c-w>");
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
