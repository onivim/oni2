
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
type direction =
  | Horizontal
  | Vertical;

[@deriving show({with_path: false})]
type split = {
  id: int,
  editorGroupId: int,
  width: option(int),
  height: option(int),
};

[@deriving show({with_path: false})]
type dockPosition =
  | Left
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
type splitTree =
  | Parent(direction, int, list(splitTree))
  | Leaf(split)
  | Empty;

[@deriving show({with_path: false})]
type t = {
  windows: splitTree,
  activeWindowId: int,
  leftDock: list(dock),
  rightDock: list(dock),
  /*
   * Cache all the registered dock items so
   * these can be reused i.e. docks can be
   * reopened and closed
   */
  dockItems: list(dock),
};

let initialWindowId = WindowId.next();

let empty = Parent(Vertical, initialWindowId, []);

let create = (): t => {
  activeWindowId: initialWindowId,
  windows: empty,
  leftDock: [],
  rightDock: [],
  dockItems: [],
};

let createSplit = (~width=?, ~height=?, ~editorGroupId, ()) => {
  id: WindowSplitId.getUniqueId(),
  editorGroupId,
  width,
  height,
};

let registerDock = (~component, ~id, ~order, ~position=Left, ~width=?, ()) => {
  order,
  component,
  position,
  id,
  width,
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

let matchingParent = (id, parentId) => id == parentId;

let getRootId = (currentTree) => {
  switch (currentTree) {
  | Parent(_, id, _) => id
  | Leaf({ id, _}) => id
  | Empty => -1
  }
}

let rec addSplit = (rootId, direction, split, currentTree) =>
  switch (currentTree) {
  | Parent(parentDirection, parentId, tree)
      when matchingParent(rootId, parentId) && parentDirection != direction && List.length(tree) == 0 =>
    Parent(direction, parentId, [Leaf(split)]);
  | Parent(parentDirection, parentId, tree)
      when matchingParent(rootId, parentId) && parentDirection != direction =>
    let newParentId = WindowId.next();
    let newParent =
      Parent(
        direction,
        newParentId,
        [Leaf(split)],
      );
    Parent(direction, parentId, tree @ [newParent]);
  | Parent(parentDirection, parentId, children) when matchingParent(parentId, rootId) =>
    Parent(
      parentDirection,
      parentId,
      [Leaf(split), ...children],
    )
  | Parent(parentDirection, parentId, children) =>
    let newChildren =
      List.map(child => addSplit(rootId, direction, split, child), children);
    Parent(parentDirection, parentId, newChildren);
  | Leaf(split) => Leaf(split)
  | Empty => Empty
  };

let rec removeSplit = (id, currentTree) =>
  switch (currentTree) {
  | Parent(direction, parentId, children) =>
    let newChildren = List.map(child => removeSplit(id, child), children);
    Parent(direction, parentId, newChildren);
  | Leaf(split) when split.id == id => Empty
  | Leaf(_) as leaf => leaf
  | Empty => Empty
  };
