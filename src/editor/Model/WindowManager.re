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

[@deriving show({with_path: false})]
type dock = {
  component: componentCreator,
  width: option(int),
  height: option(int),
};

[@deriving show({with_path: false})]
type splitTree =
  | Parent(direction, int, list(splitTree))
  | Leaf(split);

[@deriving show({with_path: false})]
type splits = splitTree;

type t = {
  windows: splits,
  leftDock: option(dock),
  rightDock: option(dock),
};

let empty = Parent(Horizontal, 0, []);

let create = (): t => {windows: empty, leftDock: None, rightDock: None};

let getId = (id: option(int)) =>
  switch (id) {
  | Some(i) => i
  | None => WindowSplitId.getUniqueId()
  };

let createSplit =
    (~id=?, ~parentId, ~width=?, ~height=?, ~component, ~direction, ()) => {
  id: getId(id),
  parentId,
  component,
  width,
  height,
  direction,
};

let createDock = (~width=?, ~height=?, ~component, ()) => {
  width,
  height,
  component,
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

let rec add = (id, split, tree) =>
  switch (tree) {
  | Parent(direction, parentId, children) when id == parentId =>
    Parent(direction, parentId, [Leaf(split), ...children])
  | Parent(direction, parentId, children) =>
    let newChildren = List.map(child => add(id, split, child), children);
    Parent(direction, parentId, newChildren);
  | Leaf(_) => tree
  };
