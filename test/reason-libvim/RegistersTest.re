
//open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = s => ignore(Vim.input(s));

describe("Registers", ({test, _}) => {
    test("get unset register", ({expect, _}) => {
      let _ = resetBuffer();
      let register = Registers.get(~register='Z');
      expect.option(register).toBeNone();
    });
    test("set register - single line", ({expect, _}) => {
      let _ = resetBuffer();
      input("\"dyy");
      let register = Registers.get(~register='d');
      expect.equal(register, Some([|"This is the first line of a test file"|]));
    });
    test("set register - multiple lines", ({expect, _}) => {
      let _ = resetBuffer();
      input("gg");
      input("\"dyG");
      let register = Registers.get(~register='d');
      expect.equal(register, Some([|
      "This is the first line of a test file",
      "This is the second line of a test file",
      "This is the third line of a test file"
      |]));
    });
});
