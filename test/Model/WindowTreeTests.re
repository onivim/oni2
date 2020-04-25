open TestFramework;

/* open Helpers; */

module Layout = Feature_Layout;

module DSL = {
  open Layout;

  let hsplit = (~weight=1., children) =>
    Split(`Horizontal, Weight(weight), children);
  let vsplit = (~weight=1., children) =>
    Split(`Vertical, Weight(weight), children);
  let window = (~weight=1., id) => Window(Weight(weight), id);
};

include DSL;

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

      expect.equal(window(1), newLayout);
    })
  });

  describe("addWindow", ({test, _}) => {
    test("add vertical split", ({expect, _}) => {
      let layout = Layout.initial;

      expect.equal(vsplit([]), layout);

      let layout =
        Layout.addWindow(
          ~target=None,
          ~position=`Before,
          `Vertical,
          1,
          layout,
        );

      expect.equal(window(1), layout);

      let layout =
        Layout.addWindow(
          ~target=Some(1),
          ~position=`Before,
          `Vertical,
          2,
          layout,
        );
      expect.equal(vsplit([window(2), window(1)]), layout);
    });

    test("add vertical split - after", ({expect, _}) => {
      let layout = Layout.initial;

      expect.equal(vsplit([]), layout);

      let layout =
        Layout.addWindow(
          ~target=None,
          ~position=`After,
          `Vertical,
          1,
          layout,
        );

      expect.equal(window(1), layout);

      let layout =
        Layout.addWindow(
          ~target=Some(1),
          ~position=`After,
          `Vertical,
          2,
          layout,
        );
      expect.equal(vsplit([window(1), window(2)]), layout);
    });

    test("parent split changes direction if needed", ({expect, _}) => {
      let layout = Layout.initial;

      expect.equal(vsplit([]), layout);

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

      expect.equal(hsplit([window(2), window(1)]), layout);

      let layout =
        Layout.addWindow(
          ~target=Some(1),
          ~position=`Before,
          `Vertical,
          3,
          layout,
        );

      expect.equal(
        hsplit([window(2), vsplit([window(3), window(1)])]),
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

    expect.equal(vsplit([window(3), window(2), window(1)]), tree);

    let newTree = Layout.rotateForward(3, tree);

    expect.equal(vsplit([window(1), window(3), window(2)]), newTree);
  });

  test("nested tree", ({expect, _}) => {
    let tree =
      vsplit([window(4), hsplit([window(3), window(2), window(1)])]);

    let newTree = Layout.rotateForward(1, tree);

    expect.equal(
      vsplit([window(4), hsplit([window(1), window(3), window(2)])]),
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

    expect.equal(vsplit([window(3), window(2), window(1)]), tree);

    let newTree = Layout.rotateBackward(3, tree);

    expect.equal(vsplit([window(2), window(1), window(3)]), newTree);
  });

  test("nested tree", ({expect, _}) => {
    let tree =
      vsplit([window(4), hsplit([window(3), window(2), window(1)])]);

    let newTree = Layout.rotateBackward(1, tree);

    expect.equal(
      vsplit([window(4), hsplit([window(2), window(1), window(3)])]),
      newTree,
    );
  });
});

describe("resizeWindow", ({test, _}) => {
  test("vsplit  - vresize", ({expect, _}) => {
    let tree = vsplit([window(1), window(2)]);

    let newTree = Layout.resizeWindow(`Vertical, 2, 5., tree);

    expect.equal(vsplit([window(1), window(2)]), newTree);
  });

  test("vsplit  - hresize", ({expect, _}) => {
    let tree = vsplit([window(1), window(2)]);

    let newTree = Layout.resizeWindow(`Horizontal, 2, 5., tree);

    expect.equal(vsplit([window(1), window(~weight=5., 2)]), newTree);
  });

  test("hsplit  - hresize", ({expect, _}) => {
    let tree = hsplit([window(1), window(2)]);

    let newTree = Layout.resizeWindow(`Horizontal, 2, 5., tree);

    expect.equal(hsplit([window(1), window(2)]), newTree);
  });

  test("hsplit  - vresize", ({expect, _}) => {
    let tree = hsplit([window(1), window(2)]);

    let newTree = Layout.resizeWindow(`Vertical, 2, 5., tree);

    expect.equal(hsplit([window(1), window(~weight=5., 2)]), newTree);
  });

  test("vsplit+hsplit - hresize", ({expect, _}) => {
    let tree = vsplit([window(1), hsplit([window(2), window(3)])]);

    let newTree = Layout.resizeWindow(`Horizontal, 2, 5., tree);

    expect.equal(
      vsplit([window(1), hsplit(~weight=5., [window(2), window(3)])]),
      newTree,
    );
  });

  test("vsplit+hsplit - vresize", ({expect, _}) => {
    let tree = vsplit([window(1), hsplit([window(2), window(3)])]);

    let newTree = Layout.resizeWindow(`Vertical, 2, 5., tree);

    expect.equal(
      vsplit([window(1), hsplit([window(~weight=5., 2), window(3)])]),
      newTree,
    );
  });

  test("hsplit+vsplit - hresize", ({expect, _}) => {
    let tree = hsplit([window(1), vsplit([window(2), window(3)])]);

    let newTree = Layout.resizeWindow(`Horizontal, 2, 5., tree);

    expect.equal(
      hsplit([window(1), vsplit([window(~weight=5., 2), window(3)])]),
      newTree,
    );
  });

  test("hsplit+vsplit - vresize", ({expect, _}) => {
    let tree = hsplit([window(1), vsplit([window(2), window(3)])]);

    let newTree = Layout.resizeWindow(`Vertical, 2, 5., tree);

    expect.equal(
      hsplit([window(1), vsplit(~weight=5., [window(2), window(3)])]),
      newTree,
    );
  });
});
