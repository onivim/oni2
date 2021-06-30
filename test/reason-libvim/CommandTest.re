open TestFramework;

let reset = () => Helpers.resetBuffer("test/reason-libvim/testfile.txt");

describe("Command", ({describe, _}) => {
  describe("output", ({test, _}) => {
    test("output is produced", ({expect, _}) => {
      let _ = reset();

      let (_context, effects) = Vim.command("!echo 'hi'");

      let matchingEffect =
        effects
        |> List.filter(
             fun
             | Vim.Effect.Output({isSilent, _}) when isSilent == false => true
             | _ => false,
           );

      expect.equal(List.length(matchingEffect), 1);
    });

    test("output is produced, respecting silent flag", ({expect, _}) => {
      let _ = reset();

      let (_context, effects) = Vim.command("silent !echo 'hi'");

      let matchingEffect =
        effects
        |> List.filter(
             fun
             | Vim.Effect.Output({isSilent, _}) when isSilent == true => true
             | _ => false,
           );

      expect.equal(List.length(matchingEffect), 1);
    });
  });
  describe("vimCommands", ({test, _}) => {
    test("define multi-line function", ({expect, _}) => {
      let _ = reset();

      let (_context, _effects) =
        Vim.commands([|
          "function! _VimBoxShellSlash()",
          "  return 42",
          "endfunction",
        |]);

      expect.equal(Vim.eval("_VimBoxShellSlash()"), Ok("42"));
    });
    test("define multi-line function", ({expect, _}) => {
      let _ = reset();

      let (_context, _effects) =
        Vim.commands([|
          "function! _VimBoxShellSlash()",
          "  return 42",
          "endfunction",
          "function! _AnotherFunction()",
          "  return 99",
          "endfunction",
        |]);

      expect.equal(Vim.eval("_AnotherFunction()"), Ok("99"));
    });
  });
});
