open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = s => ignore(Vim.input(s));
let key = s => ignore(Vim.key(s));

describe("InsertModeEdit", ({describe, _}) => {
  describe("utf8", ({test, _}) => {
    test("insert 32773", ({expect, _}) => {
      let buffer = resetBuffer();
      let charToInsert = Zed_utf8.singleton(Uchar.of_int(32773));
      input("O");
      input(charToInsert);
      let line = Buffer.getLine(buffer, LineNumber.zero);
      expect.string(line).toEqual(charToInsert);
    });
    test("insert κόσμε", ({expect, _}) => {
      let buffer = resetBuffer();
      input("O");
      input("κόσμε");
      let line = Buffer.getLine(buffer, LineNumber.zero);
      expect.string(line).toEqual("κόσμε");
    });
  });
  describe("insert mode", ({test, _}) => {
    test("insert mode should flip modified flag", ({expect, _}) => {
      let buffer = resetBuffer();

      expect.bool(Buffer.isModified(buffer)).toBe(false);

      input("I");
      let line = Buffer.getLine(buffer, LineNumber.zero);
      expect.string(line).toEqual("This is the first line of a test file");

      expect.bool(Buffer.isModified(buffer)).toBe(false);

      input("z");
      let line = Buffer.getLine(buffer, LineNumber.zero);
      expect.string(line).toEqual("zThis is the first line of a test file");

      expect.bool(Buffer.isModified(buffer)).toBe(true);
    });
    test("getLine reflects inserted text", ({expect, _}) => {
      let buffer = resetBuffer();

      input("I");
      let line = Buffer.getLine(buffer, LineNumber.zero);
      expect.string(line).toEqual("This is the first line of a test file");

      input("a");
      let line = Buffer.getLine(buffer, LineNumber.zero);
      expect.string(line).toEqual("aThis is the first line of a test file");

      input("b");
      let line = Buffer.getLine(buffer, LineNumber.zero);
      expect.string(line).toEqual("abThis is the first line of a test file");

      input("c");
      let line = Buffer.getLine(buffer, LineNumber.zero);
      expect.string(line).toEqual(
        "abcThis is the first line of a test file",
      );

      key("<cr>");
      let line = Buffer.getLine(buffer, LineNumber.zero);
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
  });
  describe("count", ({test, _}) => {
    test("count + i", ({expect, _}) => {
      let buffer = resetBuffer();

      input("5");
      input("i");
      input("abc");
      key("<esc>");

      let line = Buffer.getLine(buffer, LineNumber.zero);
      expect.string(line).toEqual(
        "abcabcabcabcabcThis is the first line of a test file",
      );
    })
  });
});
