open TestFramework;

module Selection = Oni_Components.Selection;

let testString = "Some Strin";
let creator = Selection.create(testString);

describe("Selection#initial", ({test, _}) => {
  test("Returns initial value", ({expect}) => {
    let result = Selection.initial;

    expect.int(result.anchor).toBe(0);
    expect.int(result.focus).toBe(0);
  });
});

describe("Selection#create", ({test, _}) => {
  test("Returns valid selection", ({expect}) => {
    let result = creator(~anchor=3, ~focus=3);

    expect.int(result.anchor).toBe(3);
    expect.int(result.focus).toBe(3);
  });

  test("Handle different values", ({expect}) => {
    let result = creator(~anchor=3, ~focus=6);

    expect.int(result.anchor).toBe(3);
    expect.int(result.focus).toBe(6);
  });

  test("Handle values below 0", ({expect}) => {
    let result = creator(~anchor=-1, ~focus=-3);

    expect.int(result.anchor).toBe(0);
    expect.int(result.focus).toBe(0);
  });

  test("Handle above length", ({expect}) => {
    let result = creator(~anchor=70, ~focus=10);

    expect.int(result.anchor).toBe(10);
    expect.int(result.focus).toBe(10);
  });
});

describe("Selection#anchor", ({test, _}) => {
  test("Returns anchor value", ({expect}) => {
    let result = Selection.anchor(creator(~anchor=3, ~focus=5));

    expect.int(result).toBe(3);
  });
});

describe("Selection#focus", ({test, _}) => {
  test("Returns anchor value", ({expect}) => {
    let result = Selection.focus(creator(~anchor=3, ~focus=5));

    expect.int(result).toBe(5);
  });
});

describe("Selection#range", ({test, _}) => {
  test("Returns range when anchor comes first", ({expect}) => {
    let result = Selection.range(creator(~anchor=3, ~focus=5));

    expect.int(result).toBe(2);
  });

  test("Returns range when anchor comes last", ({expect}) => {
    let result = Selection.range(creator(~anchor=5, ~focus=3));

    expect.int(result).toBe(2);
  });

  test("Returns 0 for collapsed selection", ({expect}) => {
    let result = Selection.range(creator(~anchor=3, ~focus=3));

    expect.int(result).toBe(0);
  });
});

describe("Selection#rangeStart", ({test, _}) => {
  test("Returns anchor", ({expect}) => {
    let result = Selection.rangeStart(creator(~anchor=3, ~focus=5));

    expect.int(result).toBe(3);
  });

  test("Returns focus", ({expect}) => {
    let result = Selection.rangeStart(creator(~anchor=5, ~focus=3));

    expect.int(result).toBe(3);
  });

  test("Returns any", ({expect}) => {
    let result = Selection.rangeStart(creator(~anchor=3, ~focus=3));

    expect.int(result).toBe(3);
  });
});

describe("Selection#rangeEnd", ({test, _}) => {
  test("Returns anchor", ({expect}) => {
    let result = Selection.rangeEnd(creator(~anchor=5, ~focus=3));

    expect.int(result).toBe(5);
  });

  test("Returns focus", ({expect}) => {
    let result = Selection.rangeEnd(creator(~anchor=3, ~focus=5));

    expect.int(result).toBe(5);
  });

  test("Returns any", ({expect}) => {
    let result = Selection.rangeEnd(creator(~anchor=3, ~focus=3));

    expect.int(result).toBe(3);
  });
});

describe("Selection#isCollapsed", ({test, _}) => {
  test("Returns true", ({expect}) => {
    let result = Selection.isCollapsed(creator(~anchor=3, ~focus=3));

    expect.bool(result).toBe(true);
  });

  test("Returns false", ({expect}) => {
    let result = Selection.isCollapsed(creator(~anchor=3, ~focus=7));

    expect.bool(result).toBe(false);
  });
});

describe("Selection#collapse", ({describe, _}) => {
  describe("Relatively left", ({test, _}) => {
    test("Collapse when no selection", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=3, ~focus=3), Left(testString, 2));

      expect.equal(result, creator(~anchor=1, ~focus=1))
    });

    test("Collapse selected when focus to the right", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=7, ~focus=3), Left(testString, 2));

      expect.equal(result, creator(~anchor=1, ~focus=1))
    });

    test("Collapse selected when focus to the left", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=3, ~focus=7), Left(testString, 2));

      expect.equal(result, creator(~anchor=1, ~focus=1))
    });

    test("Collapse with 0 offset", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=3, ~focus=3), Left(testString, 0));

      expect.equal(result, creator(~anchor=3, ~focus=3))
    });

    test("Doesn't exceed 0", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=3, ~focus=3), Left(testString, 15));

      expect.equal(result, creator(~anchor=0, ~focus=0))
    });
  });

  describe("Relatively Right", ({test, _}) => {
    test("Collapse when no selection", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=3, ~focus=3), Right(testString, 2));

      expect.equal(result, creator(~anchor=5, ~focus=5))
    });

    test("Collapse selected when focus to the right", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=7, ~focus=3), Right(testString, 2));

      expect.equal(result, creator(~anchor=9, ~focus=9))
    });

    test("Collapse selected when focus to the left", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=3, ~focus=7), Right(testString, 2));

      expect.equal(result, creator(~anchor=9, ~focus=9))
    });

    test("Collapse with 0 offset", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=3, ~focus=3), Right(testString, 0));

      expect.equal(result, creator(~anchor=3, ~focus=3))
    });

    test("Doesn't exceed length", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=3, ~focus=3), Right(testString, 15));

      expect.equal(result, creator(10, 10))
    });
  });

  describe("To the Start", ({test, _}) => {
    test("Collapse to the Start without selection", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=3, ~focus=3), Start);

      expect.equal(result, creator(~anchor=0, ~focus=0))
    });

    test("Collapse to the Start with selection", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=3, ~focus=7), Start);

      expect.equal(result, creator(~anchor=0, ~focus=0))
    });
  });

  describe("To the End", ({test, _}) => {
    test("Collapse to the Start without selection", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=3, ~focus=3), End(testString));

      expect.equal(result, creator(10, 10))
    });

    test("Collapse to the Start with selection", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=3, ~focus=7), End(testString));

      expect.equal(result, creator(10, 10))
    });
  });

  describe("Absolute Position", ({test, _}) => {
    test("Collapse with selection", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=3, ~focus=7), Position(testString, 8));

      expect.equal(result, creator(~anchor=8, ~focus=8))
    });

    test("Collapse without selection", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=5, ~focus=5), Position(testString, 8));

      expect.equal(result, creator(~anchor=8, ~focus=8))
    });

    test("Doesn't exceed 0", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=3, ~focus=7), Position(testString, -6));

      expect.equal(result, creator(~anchor=0, ~focus=0))
    });

    test("Doesn't exceed length", ({expect}) => {
      let result = Selection.collapse(creator(~anchor=3, ~focus=7), Position(testString, 80));

      expect.equal(result, creator(10, 10))
    });
  });
});

describe("Selection#extend", ({describe, _}) => {
  describe("Relatively Left", ({test, _}) => {
    test("Extends when focus to the left", ({expect}) => {
      let result = Selection.extend(creator(~anchor=7, ~focus=3), Left(testString, 2));

      expect.equal(result, creator(~anchor=7, ~focus=1))
    });

    test("Extends when focus to the right", ({expect}) => {
      let result = Selection.extend(creator(~anchor=3, ~focus=7), Left(testString, 2));

      expect.equal(result, creator(~anchor=3, ~focus=5))
    });

    test("Get over the anchor", ({expect}) => {
      let result = Selection.extend(creator(~anchor=3, ~focus=7), Left(testString, 5));

      expect.equal(result, creator(~anchor=3, ~focus=2))
    });

    test("Doesn't move when extends for 0", ({expect}) => {
      let result = Selection.extend(creator(~anchor=7, ~focus=3), Left(testString, 0));

      expect.equal(result, creator(~anchor=7, ~focus=3))
    });

    test("Doesn't get less then 0", ({expect}) => {
      let result = Selection.extend(creator(~anchor=7, ~focus=3), Left(testString, 13));

      expect.equal(result, creator(~anchor=7, ~focus=0))
    });
  });

  describe("Relatively Right", ({test, _}) => {
    test("Extends when focus to the left", ({expect}) => {
      let result = Selection.extend(creator(~anchor=7, ~focus=3), Right(testString, 2));

      expect.equal(result, creator(~anchor=7, ~focus=5))
    });

    test("Extends when focus to the right", ({expect}) => {
      let result = Selection.extend(creator(~anchor=3, ~focus=7), Right(testString, 2));

      expect.equal(result, creator(~anchor=3, ~focus=9))
    });

    test("Get over the anchor", ({expect}) => {
      let result = Selection.extend(creator(~anchor=3, ~focus=2), Right(testString, 5));

      expect.equal(result, creator(~anchor=3, ~focus=7))
    });

    test("Doesn't move when extends for 0", ({expect}) => {
      let result = Selection.extend(creator(~anchor=7, ~focus=3), Right(testString, 0));

      expect.equal(result, creator(~anchor=7, ~focus=3))
    });

    test("Doesn't get less then 0", ({expect}) => {
      let result = Selection.extend(creator(~anchor=7, ~focus=3), Right(testString, 13));

      expect.equal(result, creator(~anchor=7, ~focus=10))
    });
  });

  describe("To the Start", ({test, _}) => {
    test("Extends when focus to the left", ({expect}) => {
      let result = Selection.extend(creator(~anchor=7, ~focus=3), Start);

      expect.equal(result, creator(~anchor=7, ~focus=0))
    });

    test("Extends when focus to the right", ({expect}) => {
      let result = Selection.extend(creator(~anchor=3, ~focus=7), Start);

      expect.equal(result, creator(~anchor=3, ~focus=0))
    });

    test("Get over the anchor", ({expect}) => {
      let result = Selection.extend(creator(~anchor=3, ~focus=2), Start);

      expect.equal(result, creator(~anchor=3, ~focus=0))
    });

    test("Collapse to the Start without selection", ({expect}) => {
      let result = Selection.extend(creator(~anchor=3, ~focus=3), Start);

      expect.equal(result, creator(~anchor=3, ~focus=0))
    });

    test("Doesn't move on the start", ({expect}) => {
      let result = Selection.extend(creator(~anchor=0, ~focus=0), Start);

      expect.equal(result, creator(~anchor=0, ~focus=0))
    });
  });

  describe("To the End", ({test, _}) => {
    test("Extends when focus to the left", ({expect}) => {
      let result = Selection.extend(creator(~anchor=7, ~focus=3), End(testString));

      expect.equal(result, creator(~anchor=7, ~focus=10))
    });

    test("Extends when focus to the right", ({expect}) => {
      let result = Selection.extend(creator(~anchor=3, ~focus=7), End(testString));

      expect.equal(result, creator(~anchor=3, ~focus=10))
    });

    test("Get over the anchor", ({expect}) => {
      let result = Selection.extend(creator(~anchor=3, ~focus=2), End(testString));

      expect.equal(result, creator(~anchor=3, ~focus=10))
    });

    test("Collapse to the End without selection", ({expect}) => {
      let result = Selection.extend(creator(~anchor=3, ~focus=3), End(testString));

      expect.equal(result, creator(~anchor=3, ~focus=10))
    });

    test("Doesn't move on the end", ({expect}) => {
      let result = Selection.extend(creator(~anchor=10, ~focus=10), End(testString));

      expect.equal(result, creator(~anchor=10, ~focus=10))
    });
  });

  describe("Absolute Position", ({test, _}) => {
    test("Extends to position", ({expect}) => {
      let result = Selection.extend(creator(~anchor=3, ~focus=7), Position(testString, 8));

      expect.equal(result, creator(~anchor=3, ~focus=8))
    });

    test("Don't get below 0", ({expect}) => {
      let result = Selection.extend(creator(~anchor=3, ~focus=7), Position(testString, -8));

      expect.equal(result, creator(~anchor=3, ~focus=0))
    });

    test("Don't get above length", ({expect}) => {
      let result = Selection.extend(creator(~anchor=3, ~focus=7), Position(testString, 77));

      expect.equal(result, creator(~anchor=3, ~focus=10))
    });
  });
});
