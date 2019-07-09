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
type docks =
  | ExplorerDock
  | MainDock;

[@deriving show({with_path: false})]
type componentCreator = unit => React.syntheticElement;

[@deriving show({with_path: false})]
type dockPosition =
  | Left
  | Right;

type direction =
  | Up
  | Left
  | Down
  | Right;

/**
   A dock is a sub type of split, it does not require
   a parent ID as docks are list of splits, not a tree
 */
[@deriving show({with_path: false})]
type dock = {
  id: docks,
  order: int,
  component: componentCreator,
  position: dockPosition,
  width: option(int),
};

[@deriving show({with_path: false})]
type t = {
  windowTree: WindowTree.t,
  activeWindowId: int,
  leftDock: list(dock),
  rightDock: list(dock),
  /*
   * Cache all the registered dock items so
   * these can be reused i.e. docks can be
   * reopened and closed
   */
  dockItems: list(dock),
  windowTreeWidth: int,
  windowTreeHeight: int,
};

let initialWindowId = WindowId.next();

let create = (): t => {
  activeWindowId: initialWindowId,
  windowTree: WindowTree.empty,
  leftDock: [],
  rightDock: [],
  dockItems: [],
  windowTreeWidth: 1,
  windowTreeHeight: 1,
};

let setTreeSize = (width, height, v: t) => {
  ...v,
  windowTreeWidth: width,
  windowTreeHeight: height,
};

let registerDock =
    (~component, ~id, ~order, ~position: dockPosition=Left, ~width=?, ()) => {
  order,
  component,
  position,
  id,
  width,
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

let removeDockItem = (~id, layout: t) => {
  let leftDock = List.filter(item => item.id != id, layout.leftDock);
  let rightDock = List.filter(item => item.id != id, layout.rightDock);
  {...layout, leftDock, rightDock};
};

let findDockItem = (id, layout: t) =>
  switch (List.find_opt(item => item.id == id, layout.dockItems)) {
  | Some(_) as item => item
  | None => None
  };
