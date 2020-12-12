open EditorCoreTypes;
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

  describe(":normal", ({test, _}) => {
    test("#941: operates across range", ({expect, _}) => {
      let buf = Helpers.resetBuffer("test/reason-libvim/testfile.txt");
      let (_: Context.t, _: list(Effect.t)) = Vim.command("1,3 norm! ^ia");

      expect.equal(
        Buffer.getLine(buf, LineNumber.zero),
        "aThis is the first line of a test file",
      );
      expect.equal(
        Buffer.getLine(buf, LineNumber.(zero + 1)),
        "aThis is the second line of a test file",
      );
      expect.equal(
        Buffer.getLine(buf, LineNumber.(zero + 2)),
        "aThis is the third line of a test file",
      );
    })
  });

  describe(":!", ({test, _}) => {
    test("output effect is produced", ({expect, _}) => {
      let (_: Context.t, effects) = Vim.command("!ls");

      Vim.Effect.(
        switch (effects) {
        | [Output({cmd, _})] => expect.equal(cmd, "ls")
        | _ => failwith("Unexpected effect")
        }
      );
    })
  });
});
