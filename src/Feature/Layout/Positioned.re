include AbstractTree;

type metadata = {
  x: int,
  y: int,
  width: int,
  height: int,
};

type t('id) = node('id, metadata);

let contains = (x, y, {meta, _}: t(_)) => {
  x >= meta.x
  && x <= meta.x
  + meta.width
  && y >= meta.y
  && y <= meta.y
  + meta.height;
};

/**
 * move
 */
let move = (targetId, dirX, dirY, node) => {
  let windows = windowNodes(node);

  let (minX, minY, maxX, maxY, deltaX, deltaY) =
    List.fold_left(
      (prev, {meta, _}) => {
        let (minX, minY, maxX, maxY, deltaX, deltaY) = prev;

        let newMinX = meta.x < minX ? meta.x : minX;
        let newMinY = meta.y < minY ? meta.y : minY;
        let newMaxX = meta.x + meta.width > maxX ? meta.x + meta.width : maxX;
        let newMaxY =
          meta.y + meta.height > maxY ? meta.y + meta.height : maxY;
        let newDeltaX = meta.width / 2 < deltaX ? meta.width / 2 : deltaX;
        let newDeltaY = meta.height / 2 < deltaY ? meta.height / 2 : deltaY;

        (newMinX, newMinY, newMaxX, newMaxY, newDeltaX, newDeltaY);
      },
      (0, 0, 1, 1, 100, 100),
      windows,
    );

  switch (List.find_opt(window => window.kind == `Window(targetId), windows)) {
  | None => None
  | Some({meta, _}) =>
    let curX = ref(meta.x + meta.width / 2);
    let curY = ref(meta.y + meta.height / 2);
    let found = ref(false);
    let result = ref(None);

    while (! found^
           && curX^ >= minX
           && curX^ < maxX
           && curY^ >= minY
           && curY^ < maxY) {
      let x = curX^;
      let y = curY^;

      switch (
        List.find_opt(
          s => s.kind != `Window(targetId) && contains(x, y, s),
          windows,
        )
      ) {
      | Some({kind: `Window(id), _}) =>
        result := Some(id);
        found := true;
      | _ => ()
      };

      curX := x + dirX * deltaX;
      curY := y + dirY * deltaY;
    };

    result^;
  };
};

let%test_module "move" =
  (module
   {
     open DSL;

     let%test_module "regression test for #603 - navigation across splits not working" =
       (module
        {
          let initial =
            hsplit(
              ~meta={x: 0, y: 0, width: 300, height: 300},
              [
                window(3, ~meta={x: 0, y: 0, width: 300, height: 100}),
                window(2, ~meta={x: 0, y: 100, width: 300, height: 100}),
                window(1, ~meta={x: 0, y: 200, width: 300, height: 100}),
              ],
            );

          let%test "3, 0, 1" = move(3, 0, 1, initial) == Some(2);
          let%test "2, 0, 1" = move(2, 0, 1, initial) == Some(1);
          let%test "1, 0, -1" = move(1, 0, -1, initial) == Some(2);
          let%test "2, 0, -1" = move(2, 0, -1, initial) == Some(3);
        });
   });

/**
 * fromLayout
 */
let rec fromLayout = (x, y, width, height, node) => {
  Layout.(
    switch (node.kind) {
    | `Split(direction, children) =>
      let totalWeight =
        children
        |> List.map(child => child.meta.size)
        |> List.fold_left((+.), 0.)
        |> max(1.);

      let sizedChildren =
        (
          switch (direction) {
          | `Horizontal =>
            let unitHeight = float(height) /. totalWeight;
            List.fold_left(
              ((y, acc), child) => {
                let height = int_of_float(unitHeight *. child.meta.size);
                let sized = fromLayout(x, y, width, height, child);
                (y + height, [sized, ...acc]);
              },
              (y, []),
              children,
            );

          | `Vertical =>
            let unitWidth = float(width) /. totalWeight;
            List.fold_left(
              ((x, acc), child) => {
                let width = int_of_float(unitWidth *. child.meta.size);
                let sized = fromLayout(x, y, width, height, child);
                (x + width, [sized, ...acc]);
              },
              (x, []),
              children,
            );
          }
        )
        |> snd
        |> List.rev;

      AbstractTree.DSL.(
        split(~meta={x, y, width, height}, direction, sizedChildren)
      );

    | `Window(id) =>
      AbstractTree.DSL.(window(~meta={x, y, width, height}, id))
    }
  );
};

let%test_module "fromLayout" =
  (module
   {
     open DSL;

     let%test "layout vertical splits" = {
       let initial = Layout.(vsplit([window(2), window(1)]));

       let actual = fromLayout(0, 0, 200, 200, initial);

       actual
       == vsplit(
            ~meta={x: 0, y: 0, width: 200, height: 200},
            [
              window(2, ~meta={x: 0, y: 0, width: 100, height: 200}),
              window(1, ~meta={x: 100, y: 0, width: 100, height: 200}),
            ],
          );
     };

     let%test "layout horizontal splits" = {
       let initial = Layout.(hsplit([window(2), window(1)]));

       let actual = fromLayout(0, 0, 200, 200, initial);

       actual
       == hsplit(
            ~meta={x: 0, y: 0, width: 200, height: 200},
            [
              window(2, ~meta={x: 0, y: 0, width: 200, height: 100}),
              window(1, ~meta={x: 0, y: 100, width: 200, height: 100}),
            ],
          );
     };

     let%test "layout mixed splits" = {
       let initial =
         Layout.(hsplit([window(2), vsplit([window(3), window(1)])]));

       let actual = fromLayout(0, 0, 200, 200, initial);

       actual
       == hsplit(
            ~meta={x: 0, y: 0, width: 200, height: 200},
            [
              window(2, ~meta={x: 0, y: 0, width: 200, height: 100}),
              vsplit(
                ~meta={x: 0, y: 100, width: 200, height: 100},
                [
                  window(3, ~meta={x: 0, y: 100, width: 100, height: 100}),
                  window(1, ~meta={x: 100, y: 100, width: 100, height: 100}),
                ],
              ),
            ],
          );
     };
   });
