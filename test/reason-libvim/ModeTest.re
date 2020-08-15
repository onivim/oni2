open TestFramework;

let resetBuffer = () => Helpers.resetBuffer("test/testfile.txt");

describe("Mode", ({describe, _}) => {
  describe("replace mode", ({test, _}) => {
    test("replace mode is reported correctly", ({expect, _}) => {
      let _ = resetBuffer();

      // Enter replace mode
      let _ = Vim.input("R");

      expect.equal(Vim.Mode.getCurrent(), Vim.Types.Replace);
    })
  });
  describe("select mode", ({test, _}) => {
    test("select mode is reported correctly", ({expect, _}) => {
      let _ = resetBuffer();

      // Enter replace mode
      let _ = Vim.input("V");
      expect.equal(Vim.Mode.getCurrent(), Vim.Types.Visual);

      let _ = Vim.key("<C-g>");

      expect.equal(Vim.Mode.getCurrent(), Vim.Types.Select);
    })
  });
});
