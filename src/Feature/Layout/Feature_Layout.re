module WindowSplitId =
  Revery.UniqueId.Make({});

type direction =
  | Up
  | Left
  | Down
  | Right;

type t = {
  windowTree: WindowTree.t,
  windowTreeWidth: int,
  windowTreeHeight: int,
};

let create = (): t => {
  windowTree: WindowTree.empty,
  windowTreeWidth: 1,
  windowTreeHeight: 1,
};

let setTreeSize = (width, height, v: t) => {
  ...v,
  windowTreeWidth: width,
  windowTreeHeight: height,
};

let moveCore = (current, dirX, dirY, model) => {
  let layout = WindowTreeLayout.layout(0, 0, 200, 200, model.windowTree);

  WindowTreeLayout.move(current, dirX, dirY, layout)
  |> Option.value(~default=current);
};

let moveLeft = current => moveCore(current, -1, 0);
let moveRight = current => moveCore(current, 1, 0);
let moveUp = current => moveCore(current, 0, -1);
let moveDown = current => moveCore(current, 0, 1);

let move = (direction: direction, current, v) => {
  switch (direction) {
  | Up => moveUp(current, v)
  | Down => moveDown(current, v)
  | Left => moveLeft(current, v)
  | Right => moveRight(current, v)
  };
};

let rotateForward = (target, model) => {
  ...model,
  windowTree: WindowTree.rotateForward(target, model.windowTree),
};

let rotateBackward = (target, model) => {
  ...model,
  windowTree: WindowTree.rotateBackward(target, model.windowTree),
};

module WindowTree = WindowTree;
module WindowTreeLayout = WindowTreeLayout;
