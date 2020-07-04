open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = s => ignore(Vim.input(s));

describe("Undo", ({describe, _}) => {
  describe("saveRegion", ({test, _}) => {
    test("undo whole buffer", ({expect, _}) => {
      let buffer = resetBuffer();

      Vim.Testing.Undo.saveRegion(0, 4);
      Buffer.setLines(~lines=[|"a", "b", "c"|], buffer);

      let index0 = Index.zero;
      let index1 = Index.(zero + 1);
      let index2 = Index.(zero + 2);

      expect.equal(Buffer.getLine(buffer, index0), "a");
      expect.equal(Buffer.getLine(buffer, index1), "b");
      expect.equal(Buffer.getLine(buffer, index2), "c");

      input("u");

      expect.equal(
        Buffer.getLine(buffer, index0),
        "This is the first line of a test file",
      );
      expect.equal(
        Buffer.getLine(buffer, index1),
        "This is the second line of a test file",
      );
      expect.equal(
        Buffer.getLine(buffer, index2),
        "This is the third line of a test file",
      );
    });

    test("undo is clamped to buffer size", ({expect, _}) => {
      let buffer = resetBuffer();
      Buffer.setLines(~lines=[|"d", "e", "f"|], buffer);

      Vim.Testing.Undo.saveRegion(-1, 5);
      Buffer.setLines(~lines=[|"a", "b", "c"|], buffer);

      let index0 = Index.zero;
      let index1 = Index.(zero + 1);
      let index2 = Index.(zero + 2);

      expect.equal(Buffer.getLine(buffer, index0), "a");
      expect.equal(Buffer.getLine(buffer, index1), "b");
      expect.equal(Buffer.getLine(buffer, index2), "c");

      input("u");

      expect.equal(Buffer.getLine(buffer, index0), "d");
      expect.equal(Buffer.getLine(buffer, index1), "e");
      expect.equal(Buffer.getLine(buffer, index2), "f");
    });

    test("undo partial buffer", ({expect, _}) => {
      let buffer = resetBuffer();

      Buffer.setLines(~lines=[|"0", "1", "2"|], buffer);
      Vim.Testing.Undo.saveRegion(0, 2);
      Buffer.setLines(~lines=[|"a", "b", "c"|], buffer);

      let index0 = Index.zero;
      let index1 = Index.(zero + 1);
      let index2 = Index.(zero + 2);

      expect.equal(Buffer.getLine(buffer, index0), "a");
      expect.equal(Buffer.getLine(buffer, index1), "b");
      expect.equal(Buffer.getLine(buffer, index2), "c");

      input("u");

      expect.equal(Buffer.getLine(buffer, index0), "0");
      expect.equal(Buffer.getLine(buffer, index1), "b");
      expect.equal(Buffer.getLine(buffer, index2), "c");
    });
  })
});
