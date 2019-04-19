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
type componentCreator = unit => React.syntheticElement;

[@deriving show({with_path: false})]
type direction =
  | Horizontal
  | Vertical;

[@deriving show({with_path: false})]
type split = {
  id: int,
  parentId: int,
  component: componentCreator,
  direction,
  /* if omitted the split will grow to occupy whatever space is available */
  width: option(int),
  height: option(int),
};

/**
   A dock is a sub type of split, it does not require
   a parent ID as docks are list of splits, not a tree
 */
[@deriving show({with_path: false})]
type dock = {
  id: int,
  component: componentCreator,
  width: option(int),
};

/**
   A partial version of a split with only the fields
   that need to be explicitly set
 */
[@deriving show({with_path: false})]
type splitMetadata = {
  component: componentCreator,
  width: option(int),
  height: option(int),
  direction,
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
};

let initialWindowId = WindowId.next();

let empty = Parent(Vertical, initialWindowId, []);

let create = (): t => {
  activeWindowId: initialWindowId,
  windows: empty,
  leftDock: [],
  rightDock: [],
};

let createSplit = (~width=?, ~height=?, ~component, ~direction, ()) => {
  component,
  width,
  height,
  direction,
};

let enrichSplit = (parentId, s: splitMetadata): split => {
  parentId,
  component: s.component,
  direction: s.direction,
  id: WindowSplitId.getUniqueId(),
  width: s.width,
  height: s.height,
};

let createDock = (~component, ~width=?, ()) => {
  component,
  id: WindowSplitId.getUniqueId(),
  width,
};

let directionChanged = (direction, split) => direction != split.direction;
let matchingParent = (id, parentId) => id == parentId;

let rec addSplit = (id, split, currentTree) =>
  switch (currentTree) {
  | Parent(direction, parentId, tree)
      when matchingParent(id, parentId) && directionChanged(direction, split) =>
    let newParentId = WindowId.next();
    let newParent =
      Parent(
        split.direction,
        newParentId,
        [Leaf(enrichSplit(newParentId, split))],
      );
    Parent(split.direction, parentId, tree @ [newParent]);
  | Parent(direction, parentId, children) when matchingParent(parentId, id) =>
    Parent(
      direction,
      parentId,
      [Leaf(enrichSplit(parentId, split)), ...children],
    )
  | Parent(direction, parentId, children) =>
    let newChildren =
      List.map(child => addSplit(id, split, child), children);
    Parent(direction, parentId, newChildren);
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
