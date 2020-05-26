open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () => Helpers.resetBuffer("test/testfile.txt");
let input = s => ignore(Vim.input(s));

describe("Options", ({describe, _}) => {
  describe("tabs / spaces", ({test, _}) => {
    test("get / set tab options", ({expect, _}) => {
      let _ = resetBuffer();

      Options.setTabSize(5);
      expect.int(Options.getTabSize()).toBe(5);

      Options.setInsertSpaces(true);
      expect.bool(Options.getInsertSpaces()).toBe(true);

      Options.setTabSize(1);
      expect.int(Options.getTabSize()).toBe(1);

      Options.setInsertSpaces(false);
      expect.bool(Options.getInsertSpaces()).toBe(false);
    });

    test("options persist when switching buffers", ({expect, _}) => {
      let b1 = resetBuffer();

      Options.setTabSize(3);
      Options.setInsertSpaces(true);

      let _ = Buffer.openFile("test/some-random-file.txt");

      Options.setTabSize(4);
      Options.setInsertSpaces(false);

      Buffer.setCurrent(b1);

      expect.int(Options.getTabSize()).toBe(3);
      expect.bool(Options.getInsertSpaces()).toBe(true);
    });

    test("insert spaces / tabs based on settings", ({expect, _}) => {
      let b = resetBuffer();

      Options.setTabSize(3);
      Options.setInsertSpaces(true);

      input("I");
      input("<tab>");

      expect.string(Buffer.getLine(b, Index.zero)).toEqual(
        "   This is the first line of a test file",
      );

      input("<bs>");

      expect.string(Buffer.getLine(b, Index.zero)).toEqual(
        "This is the first line of a test file",
      );

      Options.setTabSize(3);
      Options.setInsertSpaces(false);

      input("<tab>");
      expect.string(Buffer.getLine(b, Index.zero)).toEqual(
        "\tThis is the first line of a test file",
      );
    });
  });

  describe("line comment", ({test, _}) => {
    test("toggle comment based on settings", ({expect, _}) => {
      let b = resetBuffer();

      Options.setLineComment("; ");

      input("g");
      input("c");
      input("c");

      expect.string(Buffer.getLine(b, Index.zero)).toEqual(
        "; This is the first line of a test file",
      );

      input("g");
      input("c");
      input("c");

      expect.string(Buffer.getLine(b, Index.zero)).toEqual(
        "This is the first line of a test file",
      );

      Options.setLineComment("!!");

      input("g");
      input("c");
      input("c");

      expect.string(Buffer.getLine(b, Index.zero)).toEqual(
        "!!This is the first line of a test file",
      );
    })
  });
});
