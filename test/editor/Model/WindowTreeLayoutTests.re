open TestFramework;

/* open Helpers; */

/* open Oni_Core.Types; */
module WindowTree = Oni_Model.WindowTree;
module WindowTreeLayout = Oni_Model.WindowTreeLayout;

open WindowTree;
open WindowTreeLayout;

describe("WindowTreeLayout", ({describe, _}) =>
  describe("layout", ({test, _}) => {
    test("layout vertical splits", ({expect}) => {
      let split1 = createSplit(~editorGroupId=1, ());
      let split2 = createSplit(~editorGroupId=2, ());

      let splits =
        WindowTree.empty
        |> addSplit(~target=None, Vertical, split1)
        |> addSplit(~target=Some(split1.id), Vertical, split2);

      let layoutItems = WindowTreeLayout.layout(0, 0, 200, 200, splits);

      expect.bool(
        [
          {split: split2, width: 100, height: 200, x: 0, y: 0},
          {split: split1, width: 100, height: 200, x: 100, y: 0},
        ]
        == layoutItems,
      ).
        toBe(
        true,
      );
    });

    test("layout horizontal splits", ({expect}) => {
      let split1 = createSplit(~editorGroupId=1, ());
      let split2 = createSplit(~editorGroupId=2, ());

      let splits =
        WindowTree.empty
        |> addSplit(~target=None, Horizontal, split1)
        |> addSplit(~target=Some(split1.id), Horizontal, split2);

      let layoutItems = WindowTreeLayout.layout(0, 0, 200, 200, splits);

      expect.bool(
        [
          {split: split2, width: 200, height: 100, x: 0, y: 0},
          {split: split1, width: 200, height: 100, x: 0, y: 100},
        ]
        == layoutItems,
      ).
        toBe(
        true,
      );
    });
    test("layout mixed splits", ({expect}) => {
      let split1 = createSplit(~editorGroupId=1, ());
      let split2 = createSplit(~editorGroupId=2, ());
      let split3 = createSplit(~editorGroupId=3, ());

      let splits =
        WindowTree.empty
        |> addSplit(~target=None, Horizontal, split1)
        |> addSplit(~target=Some(split1.id), Horizontal, split2)
        |> addSplit(~target=Some(split1.id), Vertical, split3);

      let layoutItems = WindowTreeLayout.layout(0, 0, 200, 200, splits);

      expect.bool(
        [
          {split: split2, width: 200, height: 100, x: 0, y: 0},
          {split: split3, width: 100, height: 100, x: 0, y: 100},
          {split: split1, width: 100, height: 100, x: 100, y: 100},
        ]
        == layoutItems,
      ).
        toBe(
        true,
      );
    });
  })
);
