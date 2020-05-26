open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () => Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = s => ignore(Vim.input(s));

describe("InsertModeEdit", ({describe, _}) =>
  describe("insert mode", ({test, _}) => {
    test("insert mode should flip modified flag", ({expect, _}) => {
      let buffer = resetBuffer();

      expect.bool(Buffer.isModified(buffer)).toBe(false);

      input("I");
      let line = Buffer.getLine(buffer, Index.zero);
      expect.string(line).toEqual("This is the first line of a test file");

      expect.bool(Buffer.isModified(buffer)).toBe(false);

      input("z");
      let line = Buffer.getLine(buffer, Index.zero);
      expect.string(line).toEqual("zThis is the first line of a test file");

      expect.bool(Buffer.isModified(buffer)).toBe(true);
    });
    test("getLine reflects inserted text", ({expect, _}) => {
      let buffer = resetBuffer();

      input("I");
      let line = Buffer.getLine(buffer, Index.zero);
      expect.string(line).toEqual("This is the first line of a test file");

      input("a");
      let line = Buffer.getLine(buffer, Index.zero);
      expect.string(line).toEqual("aThis is the first line of a test file");

      input("b");
      let line = Buffer.getLine(buffer, Index.zero);
      expect.string(line).toEqual("abThis is the first line of a test file");

      input("c");
      let line = Buffer.getLine(buffer, Index.zero);
      expect.string(line).toEqual(
        "abcThis is the first line of a test file",
      );

      input("<cr>");
      let line = Buffer.getLine(buffer, Index.zero);
      expect.string(line).toEqual("abc");
    });
    test("changed tick should be updated after each input", ({expect, _}) => {
      let buffer = resetBuffer();

      let startChangedTick = Buffer.getVersion(buffer);
      input("I");
      let newChangedTick = Buffer.getVersion(buffer);
      expect.int(newChangedTick).toBe(startChangedTick);

      input("a");
      let newChangedTick = Buffer.getVersion(buffer);
      expect.int(newChangedTick).toBe(startChangedTick + 1);

      input("b");
      let newChangedTick = Buffer.getVersion(buffer);
      expect.int(newChangedTick).toBe(startChangedTick + 2);

      input("c");
      let newChangedTick = Buffer.getVersion(buffer);
      expect.int(newChangedTick).toBe(startChangedTick + 3);
    });
  })
);
