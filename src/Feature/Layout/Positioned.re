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

      {
        meta: {
          x,
          y,
          width,
          height,
        },
        kind: `Split((direction, sizedChildren)),
      };

    | `Window(id) => {
        meta: {
          x,
          y,
          width,
          height,
        },
        kind: `Window(id),
      }
    }
  );
};
