open TestFramework;

open Oni_Core.Utility;

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

describe("dropLast", ({test, _}) => {
  test("empty", ({expect}) =>
    expect.list(dropLast([])).toEqual([])
  );

  test("one", ({expect}) =>
    expect.list(dropLast([1])).toEqual([])
  );

  test("many", ({expect}) => {
    expect.list(dropLast([1, 2])).toEqual([1]);
    expect.list(dropLast([1, 2, 3])).toEqual([1, 2]);
  });
});

describe("StringUtil", ({describe, _}) => {
  open StringUtil;

  describe("trimLeft", ({test, _}) => {
    test("empty", ({expect}) =>
      expect.string(trimLeft("")).toEqual("")
    );

    test("all whitespace", ({expect}) =>
      expect.string(trimLeft(" ")).toEqual("")
    );

    test("no whitespace", ({expect}) =>
      expect.string(trimLeft("foo")).toEqual("foo")
    );

    test("whitespace beginning, middle and end", ({expect}) =>
      expect.string(trimLeft(" foo bar ")).toEqual("foo bar ")
    );
  });

  describe("trimRight", ({test, _}) => {
    test("empty", ({expect}) =>
      expect.string(trimRight("")).toEqual("")
    );

    test("all whitespace", ({expect}) =>
      expect.string(trimRight(" ")).toEqual("")
    );

    test("no whitespace", ({expect}) =>
      expect.string(trimRight("foo")).toEqual("foo")
    );

    test("whitespace beginning, middle and end", ({expect}) =>
      expect.string(trimRight(" foo bar ")).toEqual(" foo bar")
    );
  });
});