open TestFramework;

/* open Helpers; */

module Layout = Feature_Layout;

describe("WindowTreeTests", ({describe, _}) => {
  describe("removeWindow", ({test, _}) => {
    test("empty parent splits are removed", ({expect, _}) => {
      let layout = Layout.initial;

      let layout =
        layout
        |> Layout.addWindow(~target=None, ~position=`Before, `Vertical, 1)
        |> Layout.addWindow(
             ~target=Some(1),
             ~position=`Before,
             `Vertical,
             2,
           )
        |> Layout.addWindow(
             ~target=Some(2),
             ~position=`Before,
             `Horizontal,
             3,
           )
        |> Layout.addWindow(
             ~target=Some(1),
             ~position=`Before,
             `Horizontal,
             4,
           );

      let newLayout =
        layout
        |> Layout.removeWindow(4)
        |> Layout.removeWindow(3)
        |> Layout.removeWindow(2);

      expect.equal(
        newLayout,
        Split(
          `Vertical,
          [Split(`Horizontal, [Window({weight: 1., content: 1})])],
        ),
      );
    })
  });

  describe("addWindow", ({test, _}) => {
    test("add vertical split", ({expect, _}) => {
      let layout = Layout.initial;

      expect.equal(Layout.Split(`Vertical, [Empty]), layout);

      let layout =
        Layout.addWindow(
          ~target=None,
          ~position=`Before,
          `Vertical,
          1,
          layout,
        );

      expect.equal(
        Layout.Split(`Vertical, [Window({weight: 1., content: 1})]),
        layout,
      );

      let layout =
        Layout.addWindow(
          ~target=Some(1),
          ~position=`Before,
          `Vertical,
          2,
          layout,
        );
      expect.equal(
        Layout.Split(
          `Vertical,
          [
            Window({weight: 1., content: 2}),
            Window({weight: 1., content: 1}),
          ],
        ),
        layout,
      );
    });

    test("add vertical split - after", ({expect, _}) => {
      let layout = Layout.initial;

      expect.equal(Layout.Split(`Vertical, [Empty]), layout);

      let layout =
        Layout.addWindow(
          ~target=None,
          ~position=`After,
          `Vertical,
          1,
          layout,
        );

      expect.equal(
        Layout.Split(`Vertical, [Window({weight: 1., content: 1})]),
        layout,
      );

      let layout =
        Layout.addWindow(
          ~target=Some(1),
          ~position=`After,
          `Vertical,
          2,
          layout,
        );
      expect.equal(
        Layout.Split(
          `Vertical,
          [
            Window({weight: 1., content: 1}),
            Window({weight: 1., content: 2}),
          ],
        ),
        layout,
      );
    });

    test("parent split changes direction if needed", ({expect, _}) => {
      let layout = Layout.initial;

      expect.equal(Layout.Split(`Vertical, [Empty]), layout);

      let layout =
        Layout.addWindow(
          ~target=None,
          ~position=`Before,
          `Horizontal,
          1,
          layout,
        );
      let layout =
        Layout.addWindow(
          ~target=Some(1),
          ~position=`Before,
          `Horizontal,
          2,
          layout,
        );

      expect.equal(
        Layout.Split(
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
        layout,
      );

      let layout =
        Layout.addWindow(
          ~target=Some(1),
          ~position=`Before,
          `Vertical,
          3,
          layout,
        );

      expect.equal(
        Layout.Split(
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
        layout,
      );
    });
  });
});

describe("rotateForward", ({test, _}) => {
  test("simple tree", ({expect, _}) => {
    let tree =
      Layout.initial
      |> Layout.addWindow(~position=`Before, `Vertical, 1)
      |> Layout.addWindow(~position=`Before, `Vertical, 2)
      |> Layout.addWindow(~position=`Before, `Vertical, 3);

    expect.equal(
      Layout.Split(
        `Vertical,
        [
          Window({weight: 1., content: 3}),
          Window({weight: 1., content: 2}),
          Window({weight: 1., content: 1}),
        ],
      ),
      tree,
    );

    let newTree = Layout.rotateForward(3, tree);

    expect.equal(
      Layout.Split(
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
      Layout.initial
      |> Layout.addWindow(~position=`Before, `Vertical, 1)
      |> Layout.addWindow(
           ~position=`Before,
           `Horizontal,
           2,
           ~target=Some(1),
         )
      |> Layout.addWindow(
           ~position=`Before,
           `Horizontal,
           3,
           ~target=Some(2),
         )
      |> Layout.addWindow(~position=`Before, `Vertical, 4);

    expect.equal(
      Layout.Split(
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

    let newTree = Layout.rotateForward(1, tree);

    expect.equal(
      Layout.Split(
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
      Layout.initial
      |> Layout.addWindow(~position=`Before, `Vertical, 1)
      |> Layout.addWindow(~position=`Before, `Vertical, 2)
      |> Layout.addWindow(~position=`Before, `Vertical, 3);

    expect.equal(
      Layout.Split(
        `Vertical,
        [
          Window({weight: 1., content: 3}),
          Window({weight: 1., content: 2}),
          Window({weight: 1., content: 1}),
        ],
      ),
      tree,
    );

    let newTree = Layout.rotateBackward(3, tree);

    expect.equal(
      Layout.Split(
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
      Layout.initial
      |> Layout.addWindow(~position=`Before, `Vertical, 1)
      |> Layout.addWindow(
           ~position=`Before,
           `Horizontal,
           2,
           ~target=Some(1),
         )
      |> Layout.addWindow(
           ~position=`Before,
           `Horizontal,
           3,
           ~target=Some(2),
         )
      |> Layout.addWindow(~position=`Before, `Vertical, 4);

    expect.equal(
      Layout.Split(
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

    let newTree = Layout.rotateBackward(1, tree);

    expect.equal(
      Layout.Split(
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
