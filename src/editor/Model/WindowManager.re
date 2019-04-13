open Revery_UI;

module WindowSplitId =
  Revery.UniqueId.Make({});

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
   A partial version of split with only the fields
   a that need to be explicitly set
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
  leftDock: option(splitMetadata),
  rightDock: option(splitMetadata),
};

let empty = Parent(Vertical, 0, []);

let create = (): t => {
  activeWindowId: 0,
  windows: empty,
  leftDock: None,
  rightDock: None,
};

let createSplit = (~width=?, ~height=?, ~component, ~direction, ()) => {
  component,
  width,
  height,
  direction,
};

let createDock = createSplit(~direction=Vertical);

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

let enrichSplit = (s: splitMetadata, parentId): split => {
  parentId,
  component: s.component,
  direction: s.direction,
  id: WindowSplitId.getUniqueId(),
  width: s.width,
  height: s.height,
};

let rec add = (id, split, tree) =>
  switch (tree) {
  | Parent(direction, parentId, tree) when direction != split.direction =>
    let newParentId = parentId + 1;
    let newParent =
      Parent(
        split.direction,
        newParentId,
        [Leaf(enrichSplit(split, newParentId))],
      );
    Parent(direction, parentId, List.append(tree, [newParent]));
  | Parent(direction, parentId, children) when id == parentId =>
    Parent(
      direction,
      parentId,
      [Leaf(enrichSplit(split, parentId)), ...children],
    )
  | Parent(direction, parentId, children) =>
    let newChildren = List.map(child => add(id, split, child), children);
    Parent(direction, parentId, newChildren);
  | Leaf(_) => tree
  };
