open EditorCoreTypes;
open Vim;
open TestFramework;

open Types;

let resetBuffer = () => Helpers.resetBuffer("test/testfile.txt");
let input = s => ignore(Vim.input(s));

let makeFun = (s, _) => Some({lines: [|s|], blockType: Line});

describe("Clipboard", ({describe, _}) => {
  describe("override", ({test, _}) => {
    test("None should use default behavior", ({expect, _}) => {
      let buf = resetBuffer();

      Clipboard.setProvider(_ => None);

      input("y");
      input("y");
      input("p");

      let line = Buffer.getLine(buf, Index.fromZeroBased(1));

      expect.string(line).toEqual("This is the first line of a test file");
    });

    test("return single line", ({expect, _}) => {
      let buf = resetBuffer();

      Clipboard.setProvider(_ => Some({lines: [|"a"|], blockType: Line}));

      input("y");
      input("y");
      input("P");

      let line = Buffer.getLine(buf, Index.zero);

      expect.string(line).toEqual("a");
    });

    test("return single line, blockType character", ({expect, _}) => {
      let buf = resetBuffer();

      Clipboard.setProvider(_ =>
        Some({lines: [|"a"|], blockType: Character})
      );

      input("y");
      input("y");
      input("p");

      let line = Buffer.getLine(buf, Index.zero);

      expect.string(line).toEqual("Tahis is the first line of a test file");
    });

    test("return multiple lines, blockType character", ({expect, _}) => {
      let buf = resetBuffer();

      Clipboard.setProvider(_ =>
        Some({lines: [|"a", "b"|], blockType: Character})
      );

      input("y");
      input("y");
      input("p");

      let line0 = Buffer.getLine(buf, Index.zero);
      let line1 = Buffer.getLine(buf, Index.(zero + 1));

      expect.string(line0).toEqual("Ta");
      expect.string(line1).toEqual("bhis is the first line of a test file");
    });

    test("return multiple lines", ({expect, _}) => {
      let buf = resetBuffer();

      Clipboard.setProvider(_ =>
        Some({lines: [|"a", "b", "c"|], blockType: Line})
      );

      input("y");
      input("y");
      input("P");

      let line1 = Buffer.getLine(buf, Index.zero);
      let line2 = Buffer.getLine(buf, Index.(zero + 1));
      let line3 = Buffer.getLine(buf, Index.(zero + 2));
      let line4 = Buffer.getLine(buf, Index.(zero + 3));

      expect.string(line1).toEqual("a");
      expect.string(line2).toEqual("b");
      expect.string(line3).toEqual("c");
      expect.string(line4).toEqual("This is the first line of a test file");
    });
  });

  describe("garbage collection", ({test, _}) => {
    test("clipboard function should not get collected", ({expect, _}) => {
      let f = makeFun("a");

      let finalizeCount = ref(0);

      Clipboard.setProvider(f);

      Gc.finalise_last(() => incr(finalizeCount), f);

      Gc.full_major();

      expect.int(finalizeCount^).toBe(0);
    });

    test(
      "clipboard function should be collected if it is overridden",
      ({expect, _}) => {
      let f0 = makeFun("a");
      let f1 = makeFun("b");

      let finalizeCount = ref(0);

      Clipboard.setProvider(f0);

      Gc.finalise_last(() => incr(finalizeCount), f0);

      Clipboard.setProvider(f1);

      Gc.full_major();

      expect.int(finalizeCount^).toBe(1);
    });
  });
});
