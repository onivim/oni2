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

describe("JsonUtil", ({describe, _}) => {
  let explodedAbc =
    `Assoc([("a", `Assoc([("b", `Assoc([("c", `Int(1))]))]))]);
  let explodedAbcdef =
    `Assoc([
      (
        "a",
        `Assoc([
          ("b", `Assoc([("c", `Int(1)), ("d", `Int(2))])),
          ("e", `Assoc([("f", `Int(3))])),
        ]),
      ),
    ]);
  describe("getKeys", ({test, _}) => {
    test("simple a.b.c case", ({expect, _}) => {
      let keys = explodedAbc |> Json.getKeys;

      expect.equal(keys, ["a.b.c"]);
    });
    test("abcdef case", ({expect, _}) => {
      let keys = explodedAbcdef |> Json.getKeys;

      expect.equal(keys, ["a.b.c", "a.b.d", "a.e.f"]);
    });
  });
  describe("explode", ({test, _}) => {
    test("simple a.b.c case", ({expect, _}) => {
      let json = `Assoc([("a.b.c", `Int(1))]);

      let result = Json.explode(json);
      expect.bool(Yojson.Safe.equal(result, explodedAbc)).toBe(true);
    });
    test("nested b.c case", ({expect, _}) => {
      let json = `Assoc([("a", `Assoc([("b.c", `Int(1))]))]);

      let result = Json.explode(json);
      expect.bool(Yojson.Safe.equal(result, explodedAbc)).toBe(true);
    });
    test("multiple subkeys (abcdef)", ({expect, _}) => {
      let json =
        `Assoc([
          ("a.b.c", `Int(1)),
          ("a.b.d", `Int(2)),
          ("a.e.f", `Int(3)),
        ]);

      let result = Json.explode(json);
      expect.bool(Yojson.Safe.equal(result, explodedAbcdef)).toBe(true);
    });
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
    let text = " 123456789";

    test("empty", ({expect}) => {
      let text = "";
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=10, ~charStart=0, ~charEnd=0, text);

      expect.string(snippet).toEqual("");
      expect.int(charStart).toBe(0);
      expect.int(charEnd).toBe(0);
    });

    test("maxLength == 0", ({expect}) => {
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=0, ~charStart=0, ~charEnd=0, text);

      expect.string(snippet).toEqual("");
      expect.int(charStart).toBe(0);
      expect.int(charEnd).toBe(0);
    });

    test(
      "maxLength > length && charStart < indent | ~maxLength=10, ~charStart=0, ~charEnd=1",
      ({expect}) => {
        let (snippet, charStart, charEnd) =
          extractSnippet(~maxLength=10, ~charStart=0, ~charEnd=1, text);

        expect.string(snippet).toEqual(" 123456789");
        expect.int(charStart).toBe(0);
        expect.int(charEnd).toBe(1);
      },
    );

    test(
      "maxLength > length && charStart > indent | ~maxLength=10, ~charStart=1, ~charEnd=2",
      ({expect}) => {
        let (snippet, charStart, charEnd) =
          extractSnippet(~maxLength=10, ~charStart=1, ~charEnd=2, text);

        expect.string(snippet).toEqual("123456789");
        expect.int(charStart).toBe(0);
        expect.int(charEnd).toBe(1);
      },
    );

    test(
      "maxLength > length && charStart > charEnd | ~maxLength=10, ~charStart=1, ~charEnd=0",
      ({expect}) => {
        let (snippet, charStart, charEnd) =
          extractSnippet(~maxLength=10, ~charStart=1, ~charEnd=0, text);

        expect.string(snippet).toEqual("123456789");
        expect.int(charStart).toBe(0);
        expect.int(charEnd).toBe(-1);
      },
    );

    test(
      "maxLength < length && charStart > charEnd | ~maxLength=2, ~charStart=1, ~charEnd=0",
      ({expect}) => {
        let (snippet, charStart, charEnd) =
          extractSnippet(~maxLength=2, ~charStart=1, ~charEnd=0, text);

        expect.string(snippet).toEqual("12");
        expect.int(charStart).toBe(0);
        expect.int(charEnd).toBe(-1);
      },
    );

    test(
      "charStart > charEnd | ~maxLength=1, ~charStart=1, ~charEnd=0",
      ({expect}) => {
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=1, ~charStart=1, ~charEnd=0, text);

      expect.string(snippet).toEqual("1");
      expect.int(charStart).toBe(0);
      expect.int(charEnd).toBe(-1);
    });

    test(
      "charEnd < maxLength | ~maxLength=4, ~charStart=1, ~charEnd=3",
      ({expect}) => {
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=4, ~charStart=1, ~charEnd=3, text);

      expect.string(snippet).toEqual("1234");
      expect.int(charStart).toBe(0);
      expect.int(charEnd).toBe(2);
    });

    test(
      "charEnd > maxLength | ~maxLength=2, ~charStart=1, ~charEnd=3",
      ({expect}) => {
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=2, ~charStart=1, ~charEnd=3, text);

      expect.string(snippet).toEqual("12");
      expect.int(charStart).toBe(0);
      expect.int(charEnd).toBe(2);
    });

    test("match fits | ~maxLength=7, ~charStart=6, ~charEnd=9", ({expect}) => {
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=7, ~charStart=6, ~charEnd=9, text);

      expect.string(snippet).toEqual("...5678");
      expect.int(charStart).toBe(4);
      expect.int(charEnd).toBe(7);
    });

    test(
      "match fits, but not ellipsis | ~maxLength=4, ~charStart=3, ~charEnd=6",
      ({expect}) => {
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=4, ~charStart=3, ~charEnd=6, text);

      expect.string(snippet).toEqual("...2");
      expect.int(charStart).toBe(4);
      expect.int(charEnd).toBe(4);
    });

    test(
      "match does not fit | ~maxLength=4, ~charStart=3, ~charEnd=6",
      ({expect}) => {
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=4, ~charStart=3, ~charEnd=6, text);

      expect.string(snippet).toEqual("...2");
      expect.int(charStart).toBe(4);
      expect.int(charEnd).toBe(4);
    });

    test("real world case 1", ({expect}) => {
      let text = "// than any JS-based solution and consumes fewer resources. Repeated testing to fine tune the";
      let (snippet, charStart, charEnd) =
        extractSnippet(~maxLength=68, ~charStart=69, ~charEnd=76, text);

      expect.string(snippet).toEqual(
        "... JS-based solution and consumes fewer resources. Repeated testing",
      );
      expect.int(charStart).toBe(61);
      expect.int(charEnd).toBe(68);
    });
  });
});

let testQueue = (describe: Rely.Describe.describeFn(_), module Queue: Queue) => {
  describe("length", ({test, _}) => {
    test("empty", ({expect}) => {
      let q = Queue.empty;

      expect.int(Queue.length(q)).toBe(0);
    });

    test("push 1", ({expect}) => {
      let q = Queue.empty |> Queue.push(42);

      expect.int(Queue.length(q)).toBe(1);
    });

    test("push 4", ({expect}) => {
      let q =
        Queue.empty
        |> Queue.push(1)
        |> Queue.push(2)
        |> Queue.push(3)
        |> Queue.push(4);

      expect.int(Queue.length(q)).toBe(4);
    });

    test("pop empty", ({expect}) => {
      let (_, q) = Queue.empty |> Queue.pop;

      expect.int(Queue.length(q)).toBe(0);
    });

    test("push 1 + pop", ({expect}) => {
      let (_, q) = Queue.empty |> Queue.push(42) |> Queue.pop;

      expect.int(Queue.length(q)).toBe(0);
    });

    test("push 4 + pop", ({expect}) => {
      let (_, q) =
        Queue.empty
        |> Queue.push(1)
        |> Queue.push(2)
        |> Queue.push(3)
        |> Queue.push(4)
        |> Queue.pop;

      expect.int(Queue.length(q)).toBe(3);
    });

    test("take empty", ({expect}) => {
      let (_, q) = Queue.empty |> Queue.take(5);

      expect.int(Queue.length(q)).toBe(0);
    });
  });

  describe("isEmpty", ({test, _}) => {
    test("empty", ({expect}) => {
      let q = Queue.empty;

      expect.bool(Queue.isEmpty(q)).toBe(true);
    });

    test("push 1", ({expect}) => {
      let q = Queue.empty |> Queue.push(42);

      expect.bool(Queue.isEmpty(q)).toBe(false);
    });
  });

  describe("push", ({test, _}) => {
    test("push 1", ({expect}) => {
      let q = Queue.empty |> Queue.push(42);

      expect.list(Queue.toList(q)).toEqual([42]);
    });

    test("push 4", ({expect}) => {
      let q =
        Queue.empty
        |> Queue.push(1)
        |> Queue.push(2)
        |> Queue.push(3)
        |> Queue.push(4);

      expect.list(Queue.toList(q)).toEqual([1, 2, 3, 4]);
    });
  });

  describe("pushFront", ({test, _}) => {
    test("pushFront 1", ({expect}) => {
      let q = Queue.empty |> Queue.pushFront(42);

      expect.list(Queue.toList(q)).toEqual([42]);
    });

    test("pushFront 4", ({expect}) => {
      let q =
        Queue.empty
        |> Queue.pushFront(1)
        |> Queue.pushFront(2)
        |> Queue.pushFront(3)
        |> Queue.pushFront(4);

      expect.list(Queue.toList(q)).toEqual([4, 3, 2, 1]);
    });

    test("push 2 + pushFront 2", ({expect}) => {
      let q =
        Queue.empty
        |> Queue.push(1)
        |> Queue.push(2)
        |> Queue.pushFront(3)
        |> Queue.pushFront(4);

      expect.list(Queue.toList(q)).toEqual([4, 3, 1, 2]);
    });
  });

  describe("pop", ({test, _}) => {
    test("pop empty", ({expect}) => {
      let (maybeItem, q) = Queue.empty |> Queue.pop;

      expect.equal(maybeItem, None);
      expect.list(Queue.toList(q)).toEqual([]);
    });

    test("push 1 + pop", ({expect}) => {
      let (maybeItem, q) = Queue.empty |> Queue.push(42) |> Queue.pop;

      expect.equal(maybeItem, Some(42));
      expect.list(Queue.toList(q)).toEqual([]);
    });

    test("push 4 + pop", ({expect}) => {
      let (maybeItem, q) =
        Queue.empty
        |> Queue.push(1)
        |> Queue.push(2)
        |> Queue.push(3)
        |> Queue.push(4)
        |> Queue.pop;

      expect.equal(maybeItem, Some(1));
      expect.list(Queue.toList(q)).toEqual([2, 3, 4]);
    });

    test("push 1 + pop 2", ({expect}) => {
      let (maybeItem, q) = Queue.empty |> Queue.push(42) |> Queue.pop;

      expect.equal(maybeItem, Some(42));
      expect.list(Queue.toList(q)).toEqual([]);

      let (maybeItem, q) = Queue.pop(q);

      expect.equal(maybeItem, None);
      expect.list(Queue.toList(q)).toEqual([]);
    });
  });

  describe("take", ({test, _}) => {
    test("take empty", ({expect}) => {
      let (items, q) = Queue.empty |> Queue.take(5);

      expect.list(items).toEqual([]);
      expect.list(Queue.toList(q)).toEqual([]);
    });

    test("push 1 + take 5", ({expect}) => {
      let (items, q) = Queue.empty |> Queue.push(42) |> Queue.take(5);

      expect.list(items).toEqual([42]);
      expect.list(Queue.toList(q)).toEqual([]);
    });

    test("push 4 + take 5", ({expect}) => {
      let (items, q) =
        Queue.empty
        |> Queue.push(1)
        |> Queue.push(2)
        |> Queue.push(3)
        |> Queue.push(4)
        |> Queue.take(5);

      expect.list(items).toEqual([1, 2, 3, 4]);
      expect.list(Queue.toList(q)).toEqual([]);
    });

    test("push 4 + take 2", ({expect}) => {
      let (items, q) =
        Queue.empty
        |> Queue.push(1)
        |> Queue.push(2)
        |> Queue.push(3)
        |> Queue.push(4)
        |> Queue.take(2);

      expect.list(items).toEqual([1, 2]);
      expect.list(Queue.toList(q)).toEqual([3, 4]);
    });
  });
};

describe("Queue", ({describe, _}) => {
  testQueue(describe, (module Queue))
});

describe("ChunkyQueue", ({describe, _}) => {
  testQueue(describe, (module ChunkyQueue));

  module Queue = ChunkyQueue;

  describe("pushChunk", ({test, _}) => {
    test("empty", ({expect}) => {
      let q = Queue.empty |> Queue.pushChunk([]);

      expect.list(Queue.toList(q)).toEqual([]);
    });

    test("singleton", ({expect}) => {
      let q = Queue.empty |> Queue.pushChunk([42]);

      expect.list(Queue.toList(q)).toEqual([42]);
    });

    test("chunk", ({expect}) => {
      let q = Queue.empty |> Queue.pushChunk([1, 2, 3, 4]);

      expect.list(Queue.toList(q)).toEqual([4, 3, 2, 1]);
    });
  });

  describe("pushReversedChunk", ({test, _}) => {
    test("empty", ({expect}) => {
      let q = Queue.empty |> Queue.pushReversedChunk([]);

      expect.list(Queue.toList(q)).toEqual([]);
    });

    test("singleton", ({expect}) => {
      let q = Queue.empty |> Queue.pushReversedChunk([42]);

      expect.list(Queue.toList(q)).toEqual([42]);
    });

    test("chunk", ({expect}) => {
      let q = Queue.empty |> Queue.pushReversedChunk([1, 2, 3, 4]);

      expect.list(Queue.toList(q)).toEqual([1, 2, 3, 4]);
    });
  });
});
