/* open Oni_Core; */
open TestFramework;

/*open Oni_Core.Types;*/
module TextMateScopes = Oni_Syntax.TextMateScopes;

describe("TextMateScopes", ({test, _}) => {
  test("hello", ({expect, _}) => {
    expect.int(1).toBe(1);
  });
});
