open Vim;
open TestFramework;

describe("Eval", ({test, _}) => {
  test("Basic eval cases", ({expect, _}) => {
    expect.result(eval("2+")).toBeError();
    expect.equal(eval("2+2"), Ok("4"));
    expect.equal(eval(""), Ok(""));
  })
});
