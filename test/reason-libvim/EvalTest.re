open Vim;
open TestFramework;

describe("Eval", ({describe, test, _}) => {
  test("Basic eval cases", ({expect, _}) => {
    expect.result(eval("2+")).toBeError();
    expect.equal(eval("2+2"), Ok("4"));
    expect.equal(eval(""), Ok(""));
  });

  describe("getchar", ({test, _}) => {
    let eval = functionGetChar => {
      let context = {...Vim.Context.current(), functionGetChar};
      Vim.eval(~context);
    };
    test("Simple getchar case", ({expect, _}) => {
      expect.equal(eval(_ => 'a', "getchar()"), Ok("97"))
    });
  });
});
