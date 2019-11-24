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

  describe("extractSnippet", ({test, _}) => {
    let text = "0123456789";

    test("empty", ({expect}) => {
      let text = "";
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=0, ~charStart=0, ~charEnd=0, text);

      expect.string(snippet).toEqual("");
      expect.int(charStart).toBe(0);
      expect.int(charEnd).toBe(0);
    });

    test("maxLength > length", ({expect}) => {
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=10, ~charStart=0, ~charEnd=0, text);

      expect.string(snippet).toEqual(text);
      expect.int(charStart).toBe(0);
      expect.int(charEnd).toBe(0);
    });

    test(
      "maxLength > length && charStart > charEnd | ~maxLength=10, ~charStart=1, ~charEnd=0",
      ({expect}) => {
        let (snippet, charStart, charEnd) =
          extractSnippet(~maxLength=10, ~charStart=1, ~charEnd=0, text);

        expect.string(snippet).toEqual(text);
        expect.int(charStart).toBe(1);
        expect.int(charEnd).toBe(0);
      },
    );

    test(
      "maxLength < length && charStart > charEnd | ~maxLength=10, ~charStart=1, ~charEnd=0",
      ({expect}) => {
        let (snippet, charStart, charEnd) =
          extractSnippet(~maxLength=2, ~charStart=1, ~charEnd=0, text);

        expect.string(snippet).toEqual("01");
        expect.int(charStart).toBe(1);
        expect.int(charEnd).toBe(0);
      },
    );

    test(
      "charStart > charEnd | ~maxLength=1, ~charStart=1, ~charEnd=0",
      ({expect}) => {
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=1, ~charStart=1, ~charEnd=0, text);

      expect.string(snippet).toEqual("0");
      expect.int(charStart).toBe(1);
      expect.int(charEnd).toBe(0);
    });

    test(
      "charEnd < maxLength | ~maxLength=4, ~charStart=1, ~charEnd=3",
      ({expect}) => {
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=4, ~charStart=1, ~charEnd=3, text);

      expect.string(snippet).toEqual("0123");
      expect.int(charStart).toBe(1);
      expect.int(charEnd).toBe(3);
    });

    test(
      "charEnd > maxLength | ~maxLength=2, ~charStart=1, ~charEnd=3",
      ({expect}) => {
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=2, ~charStart=1, ~charEnd=3, text);

      expect.string(snippet).toEqual("...12");
      expect.int(charStart).toBe(3);
      expect.int(charEnd).toBe(5);
    });

    test("match fits | ~maxLength=4, ~charStart=3, ~charEnd=6", ({expect}) => {
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=4, ~charStart=3, ~charEnd=6, text);

      expect.string(snippet).toEqual("...2345");
      expect.int(charStart).toBe(4);
      expect.int(charEnd).toBe(7);
    });

    test(
      "match does not fit | ~maxLength=4, ~charStart=2, ~charEnd=8",
      ({expect}) => {
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=4, ~charStart=3, ~charEnd=6, text);

      expect.string(snippet).toEqual("...2345");
      expect.int(charStart).toBe(4);
      expect.int(charEnd).toBe(7);
    });
  });
});
