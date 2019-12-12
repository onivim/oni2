open Revery_UI;

module WindowSplitId =
  Revery.UniqueId.Make({});

module WindowId = {
  let _current = ref(0);
  let current = () => _current^;
  let next = () => {
    incr(_current);
    current();
  };
};

[@deriving show({with_path: false})]
type componentCreator = unit => React.element(React.node);

type direction =
  | Up
  | Left
  | Down
  | Right;

[@deriving show({with_path: false})]
type t = {
  windowTree: WindowTree.t,
  activeWindowId: int,
  windowTreeWidth: int,
  windowTreeHeight: int,
};

let initialWindowId = WindowId.next();

let create = (): t => {
  activeWindowId: initialWindowId,
  windowTree: WindowTree.empty,
  windowTreeWidth: 1,
  windowTreeHeight: 1,
};

let setTreeSize = (width, height, v: t) => {
  ...v,
  windowTreeWidth: width,
  windowTreeHeight: height,
};

/* Ensure the activeWindowId points to a valid winodw */
let ensureActive = (v: t) => {
  let splits: list(WindowTree.split) = WindowTree.getSplits(v.windowTree);
  let activeWindowId: int = v.activeWindowId;

  let splitIsActive =
    List.exists((s: WindowTree.split) => s.id == activeWindowId, splits);

  if (!splitIsActive && List.length(splits) > 0) {
    {...v, activeWindowId: List.hd(splits).id};
  } else {
    v;
  };
};

let moveCore = (dirX, dirY, v: t) => {
  let layout = WindowTreeLayout.layout(0, 0, 200, 200, v.windowTree);
  let newWindow = WindowTreeLayout.move(v.activeWindowId, dirX, dirY, layout);

  switch (newWindow) {
  | None => v.activeWindowId
  | Some(newId) => newId
  };
};

let moveLeft = moveCore(-1, 0);
let moveRight = moveCore(1, 0);
let moveUp = moveCore(0, -1);
let moveDown = moveCore(0, 1);

let move = (direction: direction, v) => {
  switch (direction) {
  | Up => moveUp(v)
  | Down => moveDown(v)
  | Left => moveLeft(v)
  | Right => moveRight(v)
  };
};
