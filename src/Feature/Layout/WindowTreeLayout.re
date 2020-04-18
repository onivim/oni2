open WindowTree;

[@deriving show({with_path: false})]
type t = {
  split: WindowTree.split,
  x: int,
  y: int,
  width: int,
  height: int,
};

let intersects = (x, y, split: t) => {
  x >= split.x
  && x <= split.x
  + split.width
  && y >= split.y
  && y <= split.y
  + split.height;
};

let move = (content, dirX, dirY, splits: list(t)) => {
  let (minX, minY, maxX, maxY, deltaX, deltaY) =
    List.fold_left(
      (prev, cur) => {
        let (minX, minY, maxX, maxY, deltaX, deltaY) = prev;

        let newMinX = cur.x < minX ? cur.x : minX;
        let newMinY = cur.y < minY ? cur.y : minY;
        let newMaxX = cur.x + cur.width > maxX ? cur.x + cur.width : maxX;
        let newMaxY = cur.y + cur.height > maxY ? cur.y + cur.height : maxY;
        let newDeltaX = cur.width / 2 < deltaX ? cur.width / 2 : deltaX;
        let newDeltaY = cur.height / 2 < deltaY ? cur.height / 2 : deltaY;

        (newMinX, newMinY, newMaxX, newMaxY, newDeltaX, newDeltaY);
      },
      (0, 0, 1, 1, 100, 100),
      splits,
    );

  let splitInfo = List.filter(s => s.split.editorGroupId == content, splits);

  if (List.length(splitInfo) == 0) {
    None;
  } else {
    let startSplit = List.hd(splitInfo);

    let curX = ref(startSplit.x + startSplit.width / 2);
    let curY = ref(startSplit.y + startSplit.height / 2);
    let found = ref(false);
    let result = ref(None);

    while (! found^
           && curX^ >= minX
           && curX^ < maxX
           && curY^ >= minY
           && curY^ < maxY) {
      let x = curX^;
      let y = curY^;

      let intersects =
        List.filter(
          s => s.split.id != startSplit.split.id && intersects(x, y, s),
          splits,
        );

      if (List.length(intersects) > 0) {
        result := Some(List.hd(intersects).split.editorGroupId);
        found := true;
      };

      curX := x + dirX * deltaX;
      curY := y + dirY * deltaY;
    };

    result^;
  };
};

let rec layout = (x: int, y: int, width: int, height: int, tree: WindowTree.t) => {
  switch (tree) {
  | Parent(direction, children) =>
    let startX = x;
    let startY = y;
    let count = max(List.length(children), 1);
    let individualWidth = width / count;
    let individualHeight = height / count;

    let result =
      switch (direction) {
      | Horizontal =>
        List.mapi(
          i =>
            layout(
              startX,
              startY + individualHeight * i,
              width,
              individualHeight,
            ),
          children,
        )
      | Vertical =>
        List.mapi(
          i =>
            layout(
              startX + individualWidth * i,
              startY,
              individualWidth,
              height,
            ),
          children,
        )
      };

    List.concat(result);
  | Leaf(split) => [{split, x, y, width, height}]
  | Empty => []
  };
};
