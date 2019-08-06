open TestFramework;

module Utility = Oni_Core__Utility;
open Utility;

describe("last", ({test, _}) => {
  test("empty", ({expect}) =>
    expect.bool(last([]) == None).toBe(true)
  );

  test("one", ({expect}) =>
    expect.bool(last([1]) == Some(1)).toBe(true)
  );

  test("many", ({expect}) =>
    expect.bool(last([1, 2]) == Some(2)).toBe(true)
  );
});
