open TestFramework;

open Oni_Core_Test.Helpers;

module Layout = Feature_Layout;

describe("WindowTreeLayout", ({describe, _}) => {
  describe("move", ({test, _}) =>
    test(
      "regression test for #603 - navigation across splits not working",
      ({expect, _}) => {
      let splits =
        Layout.initial
        |> Layout.addWindow(~target=None, ~position=`Before, `Horizontal, 1)
        |> Layout.addWindow(
             ~target=Some(1),
             ~position=`Before,
             `Horizontal,
             2,
           )
        |> Layout.addWindow(
             ~target=Some(2),
             ~position=`Before,
             `Horizontal,
             3,
           );

      let layoutItems = Layout.layout(0, 0, 300, 300, splits);

      expect.equal(
        Layout.{
          x: 0,
          y: 0,
          width: 300,
          height: 300,
          kind:
            `Split((
              `Horizontal,
              [
                {kind: `Window(3), x: 0, y: 0, width: 300, height: 100},
                {kind: `Window(2), x: 0, y: 100, width: 300, height: 100},
                {kind: `Window(1), x: 0, y: 200, width: 300, height: 100},
              ],
            )),
        },
        layoutItems,
      );

      let destId = Layout.Internal.move(3, 0, 1, layoutItems) |> getOrThrow;
      expect.int(destId).toBe(2);

      let destId = Layout.Internal.move(2, 0, 1, layoutItems) |> getOrThrow;
      expect.int(destId).toBe(1);

      let destId = Layout.Internal.move(1, 0, -1, layoutItems) |> getOrThrow;
      expect.int(destId).toBe(2);

      let destId = Layout.Internal.move(2, 0, -1, layoutItems) |> getOrThrow;
      expect.int(destId).toBe(3);
    })
  );

  describe("layout", ({test, _}) => {
    test("layout vertical splits", ({expect, _}) => {
      let splits =
        Layout.initial
        |> Layout.addWindow(~target=None, ~position=`Before, `Vertical, 1)
        |> Layout.addWindow(~target=Some(1), ~position=`Before, `Vertical, 2);

      let layoutItems = Layout.layout(0, 0, 200, 200, splits);

      expect.equal(
        Layout.{
          x: 0,
          y: 0,
          width: 200,
          height: 200,
          kind:
            `Split((
              `Vertical,
              [
                {kind: `Window(2), x: 0, y: 0, width: 100, height: 200},
                {kind: `Window(1), x: 100, y: 0, width: 100, height: 200},
              ],
            )),
        },
        layoutItems,
      );
    });

    test("layout horizontal splits", ({expect, _}) => {
      let splits =
        Layout.initial
        |> Layout.addWindow(~target=None, ~position=`Before, `Horizontal, 1)
        |> Layout.addWindow(
             ~target=Some(1),
             ~position=`Before,
             `Horizontal,
             2,
           );

      let layoutItems = Layout.layout(0, 0, 200, 200, splits);

      expect.equal(
        Layout.{
          x: 0,
          y: 0,
          width: 200,
          height: 200,
          kind:
            `Split((
              `Horizontal,
              [
                {kind: `Window(2), x: 0, y: 0, width: 200, height: 100},
                {kind: `Window(1), x: 0, y: 100, width: 200, height: 100},
              ],
            )),
        },
        layoutItems,
      );
    });

    test("layout mixed splits", ({expect, _}) => {
      let splits =
        Layout.initial
        |> Layout.addWindow(~target=None, ~position=`Before, `Horizontal, 1)
        |> Layout.addWindow(
             ~target=Some(1),
             ~position=`Before,
             `Horizontal,
             2,
           )
        |> Layout.addWindow(~target=Some(1), ~position=`Before, `Vertical, 3);

      let layoutItems = Layout.layout(0, 0, 200, 200, splits);

      expect.equal(
        Layout.{
          x: 0,
          y: 0,
          width: 200,
          height: 200,
          kind:
            `Split((
              `Horizontal,
              [
                {kind: `Window(2), x: 0, y: 0, width: 200, height: 100},
                {
                  x: 0,
                  y: 100,
                  width: 200,
                  height: 100,
                  kind:
                    `Split((
                      `Vertical,
                      [
                        {
                          kind: `Window(3),
                          x: 0,
                          y: 100,
                          width: 100,
                          height: 100,
                        },
                        {
                          kind: `Window(1),
                          x: 100,
                          y: 100,
                          width: 100,
                          height: 100,
                        },
                      ],
                    )),
                },
              ],
            )),
        },
        layoutItems,
      );
    });
  });
});
