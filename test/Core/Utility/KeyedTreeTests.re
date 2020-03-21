open TestFramework;

module Tree = Kernel.KeyedStringTree;

module TreeDSL = {
  let leaf = value => Tree.Leaf(value);
  let node = children =>
    Tree.Node(children |> List.to_seq |> Tree.KeyedMap.of_seq);
};

describe("KeyedTree", ({describe, _}) => {
  describe("add", ({test, _}) => {
    test("empty -> add", ({expect, _}) => {
      let initial = Tree.empty;
      let expected = TreeDSL.(node([("1", leaf(1))]));

      let actual = initial |> Tree.add(["1"], 1);

      expect.ext.tree(actual).toEqual(expected);
    });

    test("leaf -> add", ({expect, _}) => {
      let initial = TreeDSL.leaf(2);
      let expected = TreeDSL.(node([("1", leaf(1))]));

      let actual = initial |> Tree.add(["1"], 1);

      expect.ext.tree(actual).toEqual(expected);
    });

    test("node -> add", ({expect, _}) => {
      let initial = TreeDSL.(node([("2", leaf(2))]));
      let expected = TreeDSL.(node([("1", leaf(1)), ("2", leaf(2))]));

      let actual = initial |> Tree.add(["1"], 1);

      expect.ext.tree(actual).toEqual(expected);
    });
  });

  describe("get", ({test, _}) => {
    test("simple", ({expect, _}) => {
      let tree =
        TreeDSL.(
          node([
            ("1", leaf(1)),
            ("2", node([("1", leaf(21))])),
            ("3", node([("1", leaf(31)), ("2", leaf(32))])),
          ])
        );
      let expected = Some(32);

      let actual = Tree.get(Tree.path("3.2"), tree);

      expect.equal(expected, actual);
    })
  });

  describe("union", ({test, _}) => {
    let bWins = (_, _, b) => Some(b);

    test("leaf + leaf", ({expect, _}) => {
      let a = TreeDSL.leaf(1);
      let b = TreeDSL.leaf(2);
      let expected = TreeDSL.leaf(2);

      let actual = Tree.union(bWins, a, b);

      expect.ext.tree(actual).toEqual(expected);
    });
  });

  describe("map", ({test, _}) => {
    test("simple", ({expect, _}) => {
      let tree =
        TreeDSL.(
          node([
            ("1", leaf("1")),
            ("2", node([("1", leaf("21"))])),
            ("3", node([("1", leaf("31")), ("2", leaf("32"))])),
          ])
        );
      let expected =
        TreeDSL.(
          node([
            ("1", leaf(1)),
            ("2", node([("1", leaf(21))])),
            ("3", node([("1", leaf(31)), ("2", leaf(32))])),
          ])
        );

      let actual = Tree.map(int_of_string, tree);

      expect.ext.tree(actual).toEqual(expected);
    })
  });

  describe("fold", ({test, _}) => {
    test("toList", ({expect, _}) => {
      let tree =
        TreeDSL.(
          node([
            ("1", leaf(1)),
            ("2", node([("1", leaf(21))])),
            ("3", node([("1", leaf(31)), ("2", leaf(32))])),
          ])
        );
      let expected = [("3.2", 32), ("3.1", 31), ("2.1", 21), ("1", 1)];

      let actual =
        Tree.fold(
          (key, value, acc) => [(Tree.key(key), value), ...acc],
          tree,
          [],
        );

      expect.list(actual).toEqual(expected);
    })
  });
});
