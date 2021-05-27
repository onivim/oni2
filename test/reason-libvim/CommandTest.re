open EditorCoreTypes;
open Vim;
open TestFramework;

let reset = () => Helpers.resetBuffer("test/reason-libvim/testfile.txt");

describe("Command", ({describe, _}) => {
  describe("vimCommands", ({test, _}) => {
    test("search with /", ({expect, _}) => {
      let _ = reset();

      let (_context, _effects) =
        Vim.commands([|
          "function! _VimBoxShellSlash()",
          "  return 42",
          "endfunction",
        |]);

      expect.equal(Vim.eval("_VimBoxShellSlash()"), Ok("42"));
    })
  })
});
