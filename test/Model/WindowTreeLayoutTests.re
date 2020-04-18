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
      let splits =
        WindowTree.empty
        |> addSplit(~target=None, ~position=`Before, `Horizontal, 1)
        |> addSplit(~target=Some(1), ~position=`Before, `Horizontal, 2)
        |> addSplit(~target=Some(2), ~position=`Before, `Horizontal, 3);

      let layoutItems = WindowTreeLayout.layout(0, 0, 300, 300, splits);

      expect.equal(
        [
          {content: 3, width: 300, height: 100, x: 0, y: 0},
          {content: 2, width: 300, height: 100, x: 0, y: 100},
          {content: 1, width: 300, height: 100, x: 0, y: 200},
        ],
        layoutItems,
      );

      let destId = WindowTreeLayout.move(3, 0, 1, layoutItems) |> getOrThrow;
      expect.int(destId).toBe(2);

      let destId = WindowTreeLayout.move(2, 0, 1, layoutItems) |> getOrThrow;
      expect.int(destId).toBe(1);

      let destId = WindowTreeLayout.move(1, 0, -1, layoutItems) |> getOrThrow;
      expect.int(destId).toBe(2);

      let destId = WindowTreeLayout.move(2, 0, -1, layoutItems) |> getOrThrow;
      expect.int(destId).toBe(3);
    })
  );

  describe("layout", ({test, _}) => {
    test("layout vertical splits", ({expect, _}) => {
      let splits =
        WindowTree.empty
        |> addSplit(~target=None, ~position=`Before, `Vertical, 1)
        |> addSplit(~target=Some(1), ~position=`Before, `Vertical, 2);

      let layoutItems = WindowTreeLayout.layout(0, 0, 200, 200, splits);

      expect.equal(
        [
          {content: 2, width: 100, height: 200, x: 0, y: 0},
          {content: 1, width: 100, height: 200, x: 100, y: 0},
        ],
        layoutItems,
      );
    });

    test("layout horizontal splits", ({expect, _}) => {
      let splits =
        WindowTree.empty
        |> addSplit(~target=None, ~position=`Before, `Horizontal, 1)
        |> addSplit(~target=Some(1), ~position=`Before, `Horizontal, 2);

      let layoutItems = WindowTreeLayout.layout(0, 0, 200, 200, splits);

      expect.equal(
        [
          {content: 2, width: 200, height: 100, x: 0, y: 0},
          {content: 1, width: 200, height: 100, x: 0, y: 100},
        ],
        layoutItems,
      );
    });

    test("layout mixed splits", ({expect, _}) => {
      let splits =
        WindowTree.empty
        |> addSplit(~target=None, ~position=`Before, `Horizontal, 1)
        |> addSplit(~target=Some(1), ~position=`Before, `Horizontal, 2)
        |> addSplit(~target=Some(1), ~position=`Before, `Vertical, 3);

      let layoutItems = WindowTreeLayout.layout(0, 0, 200, 200, splits);

      expect.equal(
        [
          {content: 2, width: 200, height: 100, x: 0, y: 0},
          {content: 3, width: 100, height: 100, x: 0, y: 100},
          {content: 1, width: 100, height: 100, x: 100, y: 100},
        ],
        layoutItems,
      );
    });
  });
});
