open WindowTree;

[@deriving show({with_path: false})]
type t = {
  split: WindowTree.split,
  x: int,
  y: int,
  width: int,
  height: int,
};

let rec layout = (x: int, y: int, width: int, height: int, tree: WindowTree.t) => {
  switch (tree) {
  | Parent(direction, children) =>
    let startX = x;
    let startY = y;
    let count = List.length(children);
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
