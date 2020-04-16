open TestFramework;

/* open Helpers; */

module WindowTree = Feature_Layout.WindowTree;

open WindowTree;

describe("WindowTreeTests", ({describe, _}) => {
  describe("removeSplit", ({test, _}) => {
    test("empty parent splits are removed", ({expect, _}) => {
      let splits = WindowTree.empty;

      let split1 = createSplit(~editorGroupId=1, ());
      let split2 = createSplit(~editorGroupId=2, ());
      let split3 = createSplit(~editorGroupId=2, ());
      let split4 = createSplit(~editorGroupId=1, ());

      let splits =
        splits
        |> addSplit(~target=None, ~position=Before, Vertical, split1)
        |> addSplit(
             ~target=Some(split1.id),
             ~position=Before,
             Vertical,
             split2,
           )
        |> addSplit(
             ~target=Some(split2.id),
             ~position=Before,
             Horizontal,
             split3,
           )
        |> addSplit(
             ~target=Some(split1.id),
             ~position=Before,
             Horizontal,
             split4,
           );

      let newSplits =
        splits
        |> removeSplit(split4.id)
        |> removeSplit(split3.id)
        |> removeSplit(split2.id);

      expect.equal(
        newSplits,
        Parent(Vertical, [Parent(Horizontal, [Leaf(split1)])]),
      );
    })
  });
  describe("addSplit", ({test, _}) => {
    test("add vertical split", ({expect, _}) => {
      let splits = WindowTree.empty;

      expect.bool(splits == Parent(Vertical, [Empty])).toBe(true);

      let split = createSplit(~editorGroupId=1, ());
      let targetId = split.id;

      let splits =
        addSplit(~target=None, ~position=Before, Vertical, split, splits);

      expect.bool(splits == Parent(Vertical, [Leaf(split)])).toBe(true);

      let split2 = createSplit(~editorGroupId=2, ());

      let splits =
        addSplit(
          ~target=Some(targetId),
          ~position=Before,
          Vertical,
          split2,
          splits,
        );
      expect.bool(splits == Parent(Vertical, [Leaf(split2), Leaf(split)])).
        toBe(
        true,
      );
    });

    test("add vertical split - after", ({expect, _}) => {
      let splits = WindowTree.empty;

      expect.bool(splits == Parent(Vertical, [Empty])).toBe(true);

      let split = createSplit(~editorGroupId=1, ());
      let targetId = split.id;

      let splits =
        addSplit(~target=None, ~position=After, Vertical, split, splits);

      expect.bool(splits == Parent(Vertical, [Leaf(split)])).toBe(true);

      let split2 = createSplit(~editorGroupId=2, ());

      let splits =
        addSplit(
          ~target=Some(targetId),
          ~position=After,
          Vertical,
          split2,
          splits,
        );
      expect.bool(splits == Parent(Vertical, [Leaf(split), Leaf(split2)])).
        toBe(
        true,
      );
    });

    test("parent split changes direction if needed", ({expect, _}) => {
      let splits = WindowTree.empty;

      expect.bool(splits == Parent(Vertical, [Empty])).toBe(true);

      let split1 = createSplit(~editorGroupId=1, ());
      let targetId = split1.id;
      let split2 = createSplit(~editorGroupId=2, ());
      let split3 = createSplit(~editorGroupId=3, ());

      let splits =
        addSplit(~target=None, ~position=Before, Horizontal, split1, splits);
      let splits =
        addSplit(
          ~target=Some(targetId),
          ~position=Before,
          Horizontal,
          split2,
          splits,
        );

      expect.bool(
        splits
        == Parent(
             Vertical,
             [Parent(Horizontal, [Leaf(split2), Leaf(split1)])],
           ),
      ).
        toBe(
        true,
      );

      let splits =
        addSplit(
          ~target=Some(targetId),
          ~position=Before,
          Vertical,
          split3,
          splits,
        );

      expect.bool(
        splits
        == Parent(
             Vertical,
             [
               Parent(
                 Horizontal,
                 [
                   Leaf(split2),
                   Parent(Vertical, [Leaf(split3), Leaf(split1)]),
                 ],
               ),
             ],
           ),
      ).
        toBe(
        true,
      );
    });
  });
});

describe("rotateForward", ({test, _}) => {
  test("simple tree", ({expect, _}) => {
    let splitA = createSplit(~editorGroupId=1, ());
    let splitB = createSplit(~editorGroupId=2, ());
    let splitC = createSplit(~editorGroupId=3, ());
    let tree =
      WindowTree.empty
      |> addSplit(~position=Before, Vertical, splitA)
      |> addSplit(~position=Before, Vertical, splitB)
      |> addSplit(~position=Before, Vertical, splitC);

    expect.bool(
      tree == Parent(Vertical, [Leaf(splitC), Leaf(splitB), Leaf(splitA)]),
    ).
      toBe(
      true,
    );

    let newTree = rotateForward(splitC.id, tree);

    expect.bool(
      newTree
      == Parent(Vertical, [Leaf(splitA), Leaf(splitC), Leaf(splitB)]),
    ).
      toBe(
      true,
    );
  });

  test("nested tree", ({expect, _}) => {
    let splitA = createSplit(~editorGroupId=1, ());
    let splitB = createSplit(~editorGroupId=2, ());
    let splitC = createSplit(~editorGroupId=3, ());
    let splitD = createSplit(~editorGroupId=4, ());
    let tree =
      WindowTree.empty
      |> addSplit(~position=Before, Vertical, splitA)
      |> addSplit(
           ~position=Before,
           Horizontal,
           splitB,
           ~target=Some(splitA.id),
         )
      |> addSplit(
           ~position=Before,
           Horizontal,
           splitC,
           ~target=Some(splitB.id),
         )
      |> addSplit(~position=Before, Vertical, splitD);

    expect.bool(
      tree
      == Parent(
           Vertical,
           [
             Leaf(splitD),
             Parent(
               Horizontal,
               [Leaf(splitC), Leaf(splitB), Leaf(splitA)],
             ),
           ],
         ),
    ).
      toBe(
      true,
    );

    let newTree = rotateForward(splitA.id, tree);

    expect.bool(
      newTree
      == Parent(
           Vertical,
           [
             Leaf(splitD),
             Parent(
               Horizontal,
               [Leaf(splitA), Leaf(splitC), Leaf(splitB)],
             ),
           ],
         ),
    ).
      toBe(
      true,
    );
  });
});

describe("rotateBackward", ({test, _}) => {
  test("simple tree", ({expect, _}) => {
    let splitA = createSplit(~editorGroupId=1, ());
    let splitB = createSplit(~editorGroupId=2, ());
    let splitC = createSplit(~editorGroupId=3, ());
    let tree =
      WindowTree.empty
      |> addSplit(~position=Before, Vertical, splitA)
      |> addSplit(~position=Before, Vertical, splitB)
      |> addSplit(~position=Before, Vertical, splitC);

    expect.bool(
      tree == Parent(Vertical, [Leaf(splitC), Leaf(splitB), Leaf(splitA)]),
    ).
      toBe(
      true,
    );

    let newTree = rotateBackward(splitC.id, tree);

    expect.bool(
      newTree
      == Parent(Vertical, [Leaf(splitB), Leaf(splitA), Leaf(splitC)]),
    ).
      toBe(
      true,
    );
  });

  test("nested tree", ({expect, _}) => {
    let splitA = createSplit(~editorGroupId=1, ());
    let splitB = createSplit(~editorGroupId=2, ());
    let splitC = createSplit(~editorGroupId=3, ());
    let splitD = createSplit(~editorGroupId=4, ());
    let tree =
      WindowTree.empty
      |> addSplit(~position=Before, Vertical, splitA)
      |> addSplit(
           ~position=Before,
           Horizontal,
           splitB,
           ~target=Some(splitA.id),
         )
      |> addSplit(
           ~position=Before,
           Horizontal,
           splitC,
           ~target=Some(splitB.id),
         )
      |> addSplit(~position=Before, Vertical, splitD);

    expect.bool(
      tree
      == Parent(
           Vertical,
           [
             Leaf(splitD),
             Parent(
               Horizontal,
               [Leaf(splitC), Leaf(splitB), Leaf(splitA)],
             ),
           ],
         ),
    ).
      toBe(
      true,
    );

    let newTree = rotateBackward(splitA.id, tree);

    expect.bool(
      newTree
      == Parent(
           Vertical,
           [
             Leaf(splitD),
             Parent(
               Horizontal,
               [Leaf(splitB), Leaf(splitA), Leaf(splitC)],
             ),
           ],
         ),
    ).
      toBe(
      true,
    );
  });
});
