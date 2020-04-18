open TestFramework;

open Oni_Core_Test.Helpers;

module WindowTree = Feature_Layout.WindowTree;
module WindowTreeLayout = Feature_Layout.WindowTreeLayout;

open WindowTree;
open WindowTreeLayout;

describe("WindowTreeLayout", ({describe, _}) => {
  describe("move", ({test, _}) =>
    test(
      "regression test for #603 - navigation across splits not working",
      ({expect, _}) => {
      let split1 = createSplit(~editorGroupId=1, ());
      let split2 = createSplit(~editorGroupId=2, ());
      let split3 = createSplit(~editorGroupId=3, ());

      let splits =
        WindowTree.empty
        |> addSplit(~target=None, ~position=Before, Horizontal, split1)
        |> addSplit(
             ~target=Some(split1.editorGroupId),
             ~position=Before,
             Horizontal,
             split2,
           )
        |> addSplit(
             ~target=Some(split2.editorGroupId),
             ~position=Before,
             Horizontal,
             split3,
           );

      let layoutItems = WindowTreeLayout.layout(0, 0, 300, 300, splits);

      expect.equal(
        [
          {split: split3, width: 300, height: 100, x: 0, y: 0},
          {split: split2, width: 300, height: 100, x: 0, y: 100},
          {split: split1, width: 300, height: 100, x: 0, y: 200},
        ],
        layoutItems,
      );

      let destId =
        WindowTreeLayout.move(split3.editorGroupId, 0, 1, layoutItems)
        |> getOrThrow;
      expect.int(destId).toBe(split2.editorGroupId);

      let destId =
        WindowTreeLayout.move(split2.editorGroupId, 0, 1, layoutItems)
        |> getOrThrow;
      expect.int(destId).toBe(split1.editorGroupId);

      let destId =
        WindowTreeLayout.move(split1.editorGroupId, 0, -1, layoutItems)
        |> getOrThrow;
      expect.int(destId).toBe(split2.editorGroupId);

      let destId =
        WindowTreeLayout.move(split2.editorGroupId, 0, -1, layoutItems)
        |> getOrThrow;
      expect.int(destId).toBe(split3.editorGroupId);
    })
  );

  describe("layout", ({test, _}) => {
    test("layout vertical splits", ({expect, _}) => {
      let split1 = createSplit(~editorGroupId=1, ());
      let split2 = createSplit(~editorGroupId=2, ());

      let splits =
        WindowTree.empty
        |> addSplit(~target=None, ~position=Before, Vertical, split1)
        |> addSplit(
             ~target=Some(split1.editorGroupId),
             ~position=Before,
             Vertical,
             split2,
           );

      let layoutItems = WindowTreeLayout.layout(0, 0, 200, 200, splits);

      expect.equal(
        [
          {split: split2, width: 100, height: 200, x: 0, y: 0},
          {split: split1, width: 100, height: 200, x: 100, y: 0},
        ],
        layoutItems,
      );
    });

    test("layout horizontal splits", ({expect, _}) => {
      let split1 = createSplit(~editorGroupId=1, ());
      let split2 = createSplit(~editorGroupId=2, ());

      let splits =
        WindowTree.empty
        |> addSplit(~target=None, ~position=Before, Horizontal, split1)
        |> addSplit(
             ~target=Some(split1.editorGroupId),
             ~position=Before,
             Horizontal,
             split2,
           );

      let layoutItems = WindowTreeLayout.layout(0, 0, 200, 200, splits);

      expect.equal(
        [
          {split: split2, width: 200, height: 100, x: 0, y: 0},
          {split: split1, width: 200, height: 100, x: 0, y: 100},
        ],
        layoutItems,
      );
    });
    test("layout mixed splits", ({expect, _}) => {
      let split1 = createSplit(~editorGroupId=1, ());
      let split2 = createSplit(~editorGroupId=2, ());
      let split3 = createSplit(~editorGroupId=3, ());

      let splits =
        WindowTree.empty
        |> addSplit(~target=None, ~position=Before, Horizontal, split1)
        |> addSplit(
             ~target=Some(split1.editorGroupId),
             ~position=Before,
             Horizontal,
             split2,
           )
        |> addSplit(
             ~target=Some(split1.editorGroupId),
             ~position=Before,
             Vertical,
             split3,
           );

      let layoutItems = WindowTreeLayout.layout(0, 0, 200, 200, splits);

      expect.equal(
        [
          {split: split2, width: 200, height: 100, x: 0, y: 0},
          {split: split3, width: 100, height: 100, x: 0, y: 100},
          {split: split1, width: 100, height: 100, x: 100, y: 100},
        ],
        layoutItems,
      );
    });
  });
});
