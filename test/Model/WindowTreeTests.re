open TestFramework;

/* open Helpers; */

module WindowTree = Feature_Layout.WindowTree;

open WindowTree;

describe("WindowTreeTests", ({describe, _}) => {
  describe("removeSplit", ({test, _}) => {
    test("empty parent splits are removed", ({expect, _}) => {
      let splits = WindowTree.empty;

      let splits =
        splits
        |> addSplit(~target=None, ~position=`Before, `Vertical, 1)
        |> addSplit(~target=Some(1), ~position=`Before, `Vertical, 2)
        |> addSplit(~target=Some(2), ~position=`Before, `Horizontal, 3)
        |> addSplit(~target=Some(1), ~position=`Before, `Horizontal, 4);

      let newSplits =
        splits |> removeSplit(4) |> removeSplit(3) |> removeSplit(2);

      expect.equal(
        newSplits,
        Split(
          `Vertical,
          [Split(`Horizontal, [Window({weight: 1., content: 1})])],
        ),
      );
    })
  });

  describe("addSplit", ({test, _}) => {
    test("add vertical split", ({expect, _}) => {
      let splits = WindowTree.empty;

      expect.equal(Split(`Vertical, [Empty]), splits);

      let splits =
        addSplit(~target=None, ~position=`Before, `Vertical, 1, splits);

      expect.equal(
        Split(`Vertical, [Window({weight: 1., content: 1})]),
        splits,
      );

      let splits =
        addSplit(~target=Some(1), ~position=`Before, `Vertical, 2, splits);
      expect.equal(
        Split(
          `Vertical,
          [
            Window({weight: 1., content: 2}),
            Window({weight: 1., content: 1}),
          ],
        ),
        splits,
      );
    });

    test("add vertical split - after", ({expect, _}) => {
      let splits = WindowTree.empty;

      expect.equal(Split(`Vertical, [Empty]), splits);

      let splits =
        addSplit(~target=None, ~position=`After, `Vertical, 1, splits);

      expect.equal(
        Split(`Vertical, [Window({weight: 1., content: 1})]),
        splits,
      );

      let splits =
        addSplit(~target=Some(1), ~position=`After, `Vertical, 2, splits);
      expect.equal(
        Split(
          `Vertical,
          [
            Window({weight: 1., content: 1}),
            Window({weight: 1., content: 2}),
          ],
        ),
        splits,
      );
    });

    test("parent split changes direction if needed", ({expect, _}) => {
      let splits = WindowTree.empty;

      expect.equal(Split(`Vertical, [Empty]), splits);

      let splits =
        addSplit(~target=None, ~position=`Before, `Horizontal, 1, splits);
      let splits =
        addSplit(~target=Some(1), ~position=`Before, `Horizontal, 2, splits);

      expect.equal(
        Split(
          `Vertical,
          [
            Split(
              `Horizontal,
              [
                Window({weight: 1., content: 2}),
                Window({weight: 1., content: 1}),
              ],
            ),
          ],
        ),
        splits,
      );

      let splits =
        addSplit(~target=Some(1), ~position=`Before, `Vertical, 3, splits);

      expect.equal(
        Split(
          `Vertical,
          [
            Split(
              `Horizontal,
              [
                Window({weight: 1., content: 2}),
                Split(
                  `Vertical,
                  [
                    Window({weight: 1., content: 3}),
                    Window({weight: 1., content: 1}),
                  ],
                ),
              ],
            ),
          ],
        ),
        splits,
      );
    });
  });
});

describe("rotateForward", ({test, _}) => {
  test("simple tree", ({expect, _}) => {
    let tree =
      WindowTree.empty
      |> addSplit(~position=`Before, `Vertical, 1)
      |> addSplit(~position=`Before, `Vertical, 2)
      |> addSplit(~position=`Before, `Vertical, 3);

    expect.equal(
      Split(
        `Vertical,
        [
          Window({weight: 1., content: 3}),
          Window({weight: 1., content: 2}),
          Window({weight: 1., content: 1}),
        ],
      ),
      tree,
    );

    let newTree = rotateForward(3, tree);

    expect.equal(
      Split(
        `Vertical,
        [
          Window({weight: 1., content: 1}),
          Window({weight: 1., content: 3}),
          Window({weight: 1., content: 2}),
        ],
      ),
      newTree,
    );
  });

  test("nested tree", ({expect, _}) => {
    let tree =
      WindowTree.empty
      |> addSplit(~position=`Before, `Vertical, 1)
      |> addSplit(~position=`Before, `Horizontal, 2, ~target=Some(1))
      |> addSplit(~position=`Before, `Horizontal, 3, ~target=Some(2))
      |> addSplit(~position=`Before, `Vertical, 4);

    expect.equal(
      Split(
        `Vertical,
        [
          Window({weight: 1., content: 4}),
          Split(
            `Horizontal,
            [
              Window({weight: 1., content: 3}),
              Window({weight: 1., content: 2}),
              Window({weight: 1., content: 1}),
            ],
          ),
        ],
      ),
      tree,
    );

    let newTree = rotateForward(1, tree);

    expect.equal(
      Split(
        `Vertical,
        [
          Window({weight: 1., content: 4}),
          Split(
            `Horizontal,
            [
              Window({weight: 1., content: 1}),
              Window({weight: 1., content: 3}),
              Window({weight: 1., content: 2}),
            ],
          ),
        ],
      ),
      newTree,
    );
  });
});

describe("rotateBackward", ({test, _}) => {
  test("simple tree", ({expect, _}) => {
    let tree =
      WindowTree.empty
      |> addSplit(~position=`Before, `Vertical, 1)
      |> addSplit(~position=`Before, `Vertical, 2)
      |> addSplit(~position=`Before, `Vertical, 3);

    expect.equal(
      Split(
        `Vertical,
        [
          Window({weight: 1., content: 3}),
          Window({weight: 1., content: 2}),
          Window({weight: 1., content: 1}),
        ],
      ),
      tree,
    );

    let newTree = rotateBackward(3, tree);

    expect.equal(
      Split(
        `Vertical,
        [
          Window({weight: 1., content: 2}),
          Window({weight: 1., content: 1}),
          Window({weight: 1., content: 3}),
        ],
      ),
      newTree,
    );
  });

  test("nested tree", ({expect, _}) => {
    let tree =
      WindowTree.empty
      |> addSplit(~position=`Before, `Vertical, 1)
      |> addSplit(~position=`Before, `Horizontal, 2, ~target=Some(1))
      |> addSplit(~position=`Before, `Horizontal, 3, ~target=Some(2))
      |> addSplit(~position=`Before, `Vertical, 4);

    expect.equal(
      Split(
        `Vertical,
        [
          Window({weight: 1., content: 4}),
          Split(
            `Horizontal,
            [
              Window({weight: 1., content: 3}),
              Window({weight: 1., content: 2}),
              Window({weight: 1., content: 1}),
            ],
          ),
        ],
      ),
      tree,
    );

    let newTree = rotateBackward(1, tree);

    expect.equal(
      Split(
        `Vertical,
        [
          Window({weight: 1., content: 4}),
          Split(
            `Horizontal,
            [
              Window({weight: 1., content: 2}),
              Window({weight: 1., content: 1}),
              Window({weight: 1., content: 3}),
            ],
          ),
        ],
      ),
      newTree,
    );
  });
});
