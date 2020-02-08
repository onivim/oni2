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

let move = (id, dirX, dirY, splits: list(t)) => {
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

  let splitInfo = List.filter(s => s.split.id == id, splits);

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
        result := Some(List.hd(intersects).split.id);
        found := true;
      };

      curX := x + dirX * deltaX;
      curY := y + dirY * deltaY;
    };

    result^;
  };
};

let distribute = (~f, ~totalSize, children) => {
  // First, collect the total specified widths, and subtract from totalSize

  let childCount = max(List.length(children), 1);
  let calculatedSizes = Array.make(childCount, 0);

  // The number of children that have fixed sizes.
  // These don't need to be distributed over later;
  let fixedChildren = ref(0);
  let availableSize = ref(totalSize);
  List.iteri(
    (idx, child) => {
      switch (f(child, totalSize)) {
      | None => ()
      | Some(size) =>
        incr(fixedChildren);
        calculatedSizes[idx] = size;
        availableSize := availableSize^ - size;
      }
    },
    children,
  );

  let childrenToDistributeOver = childCount - fixedChildren^;
  let distributionSize =
    childrenToDistributeOver > 0
      ? availableSize^ / childrenToDistributeOver : 0;

  List.iteri(
    (idx, child) => {
      switch (f(child, totalSize)) {
      | None => calculatedSizes[idx] = distributionSize
      | Some(_) => ()
      }
    },
    children,
  );

  let calculatedSizesAndOffsets = Array.make(childCount, (0, 0));

  let idx = ref(0);
  let currentOffset = ref(0);
  while (idx^ < childCount) {
    let i = idx^;
    let currentSize = calculatedSizes[i];
    let offset = currentOffset^;
    calculatedSizesAndOffsets[i] = (currentSize, offset);
    currentOffset := offset + currentSize;
    incr(idx);
  };

  calculatedSizesAndOffsets;
};

let rec layout = (x: int, y: int, width: int, height: int, tree: WindowTree.t) => {
  switch (tree) {
  | Parent(direction, children) =>
    let startX = x;
    let startY = y;

    let distributedWidths =
      distribute(~f=(_child, _size) => None, ~totalSize=width, children);

    let distributedHeights =
      distribute(~f=(_child, _size) => None, ~totalSize=height, children);

    let result =
      switch (direction) {
      | Horizontal =>
        List.mapi(
          i => {
            let (size, offset) = distributedHeights[i];
            layout(startX, startY + offset, width, size);
          },
          children,
        )
      | Vertical =>
        List.mapi(
          i => {
            let (size, offset) = distributedWidths[i];
            layout(startX + offset, startY, size, height);
          },
          children,
        )
      };

    List.concat(result);
  | Leaf(split) => [{split, x, y, width, height}]
  | Empty => []
  };
};
