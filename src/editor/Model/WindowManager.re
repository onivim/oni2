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
  | Leaf(split);

[@deriving show({with_path: false})]
type splits = splitTree;

type t = {
  windows: splits,
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

type parentHandler('a) = (direction, 'a) => 'a;

let identity: parentHandler('a) = (_direction, result) => result;

/**
   Walk the split tree, for each parent traverse it leaves, and then pass the result
   which is an abstract type to the specified parent handler which transforms the
   eventual result. For each leaf call the action function which takes a split
 */
let rec traverseSplitTree =
        (
          ~handleParent=identity,
          ~action,
          ~result,
          ~tree,
          ~direction: direction,
          (),
        ) =>
  switch (tree) {
  | Parent(direction, _, children) =>
    List.fold_left(
      (accum, child) =>
        traverseSplitTree(
          ~action,
          ~result=accum,
          ~tree=child,
          ~handleParent,
          ~direction,
          (),
        ),
      result,
      children,
    )
    |> handleParent(direction)
  | Leaf(split) => action(result, split, direction)
  };

let rec addSplit = (id, split, currentTree) =>
  switch (currentTree) {
  | Parent(direction, parentId, tree)
      when id == parentId && direction != split.direction =>
    let newParentId = WindowId.next();
    let newParent =
      Parent(
        split.direction,
        newParentId,
        [Leaf(enrichSplit(newParentId, split))],
      );
    Parent(split.direction, parentId, tree @ [newParent]);
  | Parent(direction, parentId, children) when parentId == id =>
    Parent(
      direction,
      parentId,
      [Leaf(enrichSplit(parentId, split)), ...children],
    )
  | Parent(direction, parentId, children) =>
    let newChildren =
      List.map(child => addSplit(id, split, child), children);
    Parent(direction, parentId, newChildren);
  | Leaf(s) => Leaf(s)
  };

let rec removeSplit = (id, currentTree) =>
  switch (currentTree) {
  | Parent(direction, parentId, children) =>
    let newChildren = List.map(child => removeSplit(id, child), children);
    Parent(direction, parentId, newChildren);
  | Leaf(split) when split.id == id => currentTree
  | Leaf(_) as leaf => leaf
  };
