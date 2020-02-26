open TestFramework;

module Selection = Oni_Components.Selection;

let testString = "Some Strin";
let testStringLength = String.length(testString);
let create = Selection.create(~text=testString);

describe("Selection#initial", ({test, _}) => {
  test("Returns initial value", ({expect}) => {
    let result = Selection.initial;

    expect.int(result.anchor).toBe(0);
    expect.int(result.focus).toBe(0);
  })
});

describe("Selection#create", ({test, _}) => {
  test("Returns valid selection", ({expect}) => {
    let result = create(~anchor=3, ~focus=3);

    expect.int(result.anchor).toBe(3);
    expect.int(result.focus).toBe(3);
  });

  test("Handle different values", ({expect}) => {
    let result = create(~anchor=3, ~focus=6);

    expect.int(result.anchor).toBe(3);
    expect.int(result.focus).toBe(6);
  });

  test("Handle values below 0", ({expect}) => {
    let result = create(~anchor=-1, ~focus=-3);

    expect.int(result.anchor).toBe(0);
    expect.int(result.focus).toBe(0);
  });

  test("Handle above length", ({expect}) => {
    let result = create(~anchor=70, ~focus=55);

    expect.int(result.anchor).toBe(testStringLength);
    expect.int(result.focus).toBe(testStringLength);
  });
});

describe("Selection#anchor", ({test, _}) => {
  test("Returns anchor value", ({expect}) => {
    let result = Selection.anchor(create(~anchor=3, ~focus=5));

    expect.int(result).toBe(3);
  })
});

describe("Selection#focus", ({test, _}) => {
  test("Returns anchor value", ({expect}) => {
    let result = Selection.focus(create(~anchor=3, ~focus=5));

    expect.int(result).toBe(5);
  })
});

describe("Selection#range", ({test, _}) => {
  test("Returns range when anchor comes first", ({expect}) => {
    let result = Selection.range(create(~anchor=3, ~focus=5));

    expect.int(result).toBe(2);
  });

  test("Returns range when anchor comes last", ({expect}) => {
    let result = Selection.range(create(~anchor=5, ~focus=3));

    expect.int(result).toBe(2);
  });

  test("Returns 0 for collapsed selection", ({expect}) => {
    let result = Selection.range(create(~anchor=3, ~focus=3));

    expect.int(result).toBe(0);
  });
});

describe("Selection#rangeStart", ({test, _}) => {
  test("Returns anchor", ({expect}) => {
    let result = Selection.rangeStart(create(~anchor=3, ~focus=5));

    expect.int(result).toBe(3);
  });

  test("Returns focus", ({expect}) => {
    let result = Selection.rangeStart(create(~anchor=5, ~focus=3));

    expect.int(result).toBe(3);
  });

  test("Returns any", ({expect}) => {
    let result = Selection.rangeStart(create(~anchor=3, ~focus=3));

    expect.int(result).toBe(3);
  });
});

describe("Selection#rangeEnd", ({test, _}) => {
  test("Returns anchor", ({expect}) => {
    let result = Selection.rangeEnd(create(~anchor=5, ~focus=3));

    expect.int(result).toBe(5);
  });

  test("Returns focus", ({expect}) => {
    let result = Selection.rangeEnd(create(~anchor=3, ~focus=5));

    expect.int(result).toBe(5);
  });

  test("Returns any", ({expect}) => {
    let result = Selection.rangeEnd(create(~anchor=3, ~focus=3));

    expect.int(result).toBe(3);
  });
});

describe("Selection#isCollapsed", ({test, _}) => {
  test("Returns true", ({expect}) => {
    let result = Selection.isCollapsed(create(~anchor=3, ~focus=3));

    expect.bool(result).toBe(true);
  });

  test("Returns false", ({expect}) => {
    let result = Selection.isCollapsed(create(~anchor=3, ~focus=7));

    expect.bool(result).toBe(false);
  });
});

describe("Selection#collapse", ({test, _}) => {
  let collapse = Selection.collapse(~text=testString);

  test("Collapse selection with offset", ({expect}) => {
    let result = collapse(3);

    expect.int(result.anchor).toBe(3);
    expect.int(result.focus).toBe(3);
  });

  test("Collapse selection with offset less then 0", ({expect}) => {
    let result = collapse(-20);

    expect.int(result.anchor).toBe(0);
    expect.int(result.focus).toBe(0);
  });

  test("Collapse selection with offset is more then length", ({expect}) => {
    let result = collapse(testStringLength + 70);

    expect.int(result.anchor).toBe(testStringLength);
    expect.int(result.focus).toBe(testStringLength);
  });
});

describe("Selection#collapseRelative", ({describe, _}) => {
  let collapseRelative = Selection.collapseRelative(~text=testString);

  describe("Left", ({test, _}) => {
    test("Collapse when no selection", ({expect}) => {
      let selection = create(~anchor=3, ~focus=3);
      let result = collapseRelative(~selection, Left(2));

      expect.int(result.anchor).toBe(1);
      expect.int(result.focus).toBe(1);
    });

    test("Collapse selected when focus to the right", ({expect}) => {
      let selection = create(~anchor=7, ~focus=3);
      let result = collapseRelative(~selection, Left(2));

      expect.int(result.anchor).toBe(1);
      expect.int(result.focus).toBe(1);
    });

    test("Collapse selected when focus to the left", ({expect}) => {
      let selection = create(~anchor=3, ~focus=7);
      let result = collapseRelative(~selection, Left(2));

      expect.int(result.anchor).toBe(1);
      expect.int(result.focus).toBe(1);
    });

    test("Collapse with 0 offset", ({expect}) => {
      let selection = create(~anchor=3, ~focus=3);
      let result = collapseRelative(~selection, Left(0));

      expect.int(result.anchor).toBe(3);
      expect.int(result.focus).toBe(3);
    });

    test("Doesn't exceed 0", ({expect}) => {
      let selection = create(~anchor=3, ~focus=3);
      let result = collapseRelative(~selection, Left(15));

      expect.int(result.anchor).toBe(0);
      expect.int(result.focus).toBe(0);
    });
  });

  describe("Right", ({test, _}) => {
    test("Collapse when no selection", ({expect}) => {
      let selection = create(~anchor=3, ~focus=3);
      let result = collapseRelative(~selection, Right(2));

      expect.int(result.anchor).toBe(5);
      expect.int(result.focus).toBe(5);
    });

    test("Collapse selected when focus to the right", ({expect}) => {
      let selection = create(~anchor=7, ~focus=3);
      let result = collapseRelative(~selection, Right(2));

      expect.int(result.anchor).toBe(9);
      expect.int(result.focus).toBe(9);
    });

    test("Collapse selected when focus to the left", ({expect}) => {
      let selection = create(~anchor=3, ~focus=7);
      let result = collapseRelative(~selection, Right(2));

      expect.int(result.anchor).toBe(9);
      expect.int(result.focus).toBe(9);
    });

    test("Collapse with 0 offset", ({expect}) => {
      let selection = create(~anchor=3, ~focus=3);
      let result = collapseRelative(~selection, Right(0));

      expect.int(result.anchor).toBe(3);
      expect.int(result.focus).toBe(3);
    });

    test("Doesn't exceed length", ({expect}) => {
      let selection = create(~anchor=3, ~focus=3);
      let result = collapseRelative(~selection, Right(15));

      expect.int(result.anchor).toBe(testStringLength);
      expect.int(result.focus).toBe(testStringLength);
    });
  });
});

describe("Selection#extend", ({test, _}) => {
  let extend = Selection.extend(~text=testString);

  test("Extend when selection is collapsed", ({expect}) => {
    let selection = create(~anchor=3, ~focus=3);
    let result = extend(~selection, 5);

    expect.int(result.anchor).toBe(3);
    expect.int(result.focus).toBe(5);
  });

  test("Extend when selection is not collapsed", ({expect}) => {
    let selection = create(~anchor=3, ~focus=8);
    let result = extend(~selection, 5);

    expect.int(result.anchor).toBe(3);
    expect.int(result.focus).toBe(5);
  });

  test(
    "Doesn't extend when selection is not collapsed in offset", ({expect}) => {
    let selection = create(~anchor=3, ~focus=3);
    let result = extend(~selection, 3);

    expect.int(result.anchor).toBe(3);
    expect.int(result.focus).toBe(3);
  });

  test("Extends when offset is less than 0", ({expect}) => {
    let selection = create(~anchor=3, ~focus=3);
    let result = extend(~selection, -3);

    expect.int(result.anchor).toBe(3);
    expect.int(result.focus).toBe(0);
  });

  test("Extends when offset is more than length", ({expect}) => {
    let selection = create(~anchor=3, ~focus=3);
    let result = extend(~selection, testStringLength + 70);

    expect.int(result.anchor).toBe(3);
    expect.int(result.focus).toBe(testStringLength);
  });
});

describe("Selection#extendRelative", ({describe, _}) => {
  let extendRelative = Selection.extendRelative(~text=testString);

  describe("Left", ({test, _}) => {
    test("Extends when focus to the left", ({expect}) => {
      let selection = create(~anchor=7, ~focus=3);
      let result = extendRelative(~selection, Left(2));

      expect.int(result.anchor).toBe(7);
      expect.int(result.focus).toBe(1);
    });

    test("Extends when focus to the right", ({expect}) => {
      let selection = create(~anchor=3, ~focus=7);
      let result = extendRelative(~selection, Left(2));

      expect.int(result.anchor).toBe(3);
      expect.int(result.focus).toBe(5);
    });

    test("Get over the anchor", ({expect}) => {
      let selection = create(~anchor=3, ~focus=7);
      let result = extendRelative(~selection, Left(5));

      expect.int(result.anchor).toBe(3);
      expect.int(result.focus).toBe(2);
    });

    test("Doesn't move when extends for 0", ({expect}) => {
      let selection = create(~anchor=7, ~focus=3);
      let result = extendRelative(~selection, Left(0));

      expect.int(result.anchor).toBe(7);
      expect.int(result.focus).toBe(3);
    });

    test("Doesn't get less then 0", ({expect}) => {
      let selection = create(~anchor=7, ~focus=3);
      let result = extendRelative(~selection, Left(15));

      expect.int(result.anchor).toBe(7);
      expect.int(result.focus).toBe(0);
    });
  });

  describe("Right", ({test, _}) => {
    test("Extends when focus to the left", ({expect}) => {
      let selection = create(~anchor=7, ~focus=3);
      let result = extendRelative(~selection, Right(2));

      expect.int(result.anchor).toBe(7);
      expect.int(result.focus).toBe(5);
    });

    test("Extends when focus to the right", ({expect}) => {
      let selection = create(~anchor=3, ~focus=7);
      let result = extendRelative(~selection, Right(2));

      expect.int(result.anchor).toBe(3);
      expect.int(result.focus).toBe(9);
    });

    test("Get over the anchor", ({expect}) => {
      let selection = create(~anchor=3, ~focus=2);
      let result = extendRelative(~selection, Right(5));

      expect.int(result.anchor).toBe(3);
      expect.int(result.focus).toBe(7);
    });

    test("Doesn't move when extends for 0", ({expect}) => {
      let selection = create(~anchor=7, ~focus=3);
      let result = extendRelative(~selection, Right(0));

      expect.int(result.anchor).toBe(7);
      expect.int(result.focus).toBe(3);
    });

    test("Doesn't get less then 0", ({expect}) => {
      let selection = create(~anchor=7, ~focus=3);
      let result = extendRelative(~selection, Right(testStringLength + 20));

      expect.int(result.anchor).toBe(7);
      expect.int(result.focus).toBe(10);
    });
  });
});
