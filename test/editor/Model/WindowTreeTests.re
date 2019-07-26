open TestFramework;

/* open Helpers; */

/* open Oni_Core.Types; */
module WindowTree = Oni_Model.WindowTree;

open WindowTree;

describe("WindowTreeTests", ({describe, _}) => {
  describe("rotateSplit", ({test, _}) => {
    test("moves head split to end", ({expect}) => {
      let splits = WindowTree.empty;
      let split1 = createSplit(~editorGroupId=1, ());
      let split2 = createSplit(~editorGroupId=2, ());

      let splits = splits
        |>addSplit(~target=None, Vertical, split)
        |>addSplit(~target=Some(split1.id), Vertical, split);
 
    });
  });
  describe("addSplit", ({test, _}) => {
    test("add vertical split", ({expect}) => {
      let splits = WindowTree.empty;

      expect.bool(splits == Parent(Vertical, [Empty])).toBe(true);

      let split = createSplit(~editorGroupId=1, ());
      let targetId = split.id;

      let splits = addSplit(~target=None, Vertical, split, splits);

      expect.bool(splits == Parent(Vertical, [Leaf(split)])).toBe(true);

      let split2 = createSplit(~editorGroupId=2, ());

      let splits =
        addSplit(~target=Some(targetId), Vertical, split2, splits);
      expect.bool(splits == Parent(Vertical, [Leaf(split2), Leaf(split)])).
        toBe(
        true,
      );
    });

    test("parent split changes direction if needed", ({expect}) => {
      let splits = WindowTree.empty;

      expect.bool(splits == Parent(Vertical, [Empty])).toBe(true);

      let split1 = createSplit(~editorGroupId=1, ());
      let targetId = split1.id;
      let split2 = createSplit(~editorGroupId=2, ());
      let split3 = createSplit(~editorGroupId=3, ());

      let splits = addSplit(~target=None, Horizontal, split1, splits);
      let splits =
        addSplit(~target=Some(targetId), Horizontal, split2, splits);

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
        addSplit(~target=Some(targetId), Vertical, split3, splits);

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
  })
});
