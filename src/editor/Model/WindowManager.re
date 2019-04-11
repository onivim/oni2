open Revery_UI;

module WindowSplitId =
  Revery.UniqueId.Make({});

[@deriving show]
type componentCreator = unit => React.syntheticElement;

[@deriving show]
type direction =
  | Horizontal
  | Vertical;

[@deriving show]
type split = {
  id: int,
  parentId: int,
  component: componentCreator,
  direction,
  /* if omitted the split will grow to occupy whatever space is available */
  width: option(int),
  height: option(int),
};

[@deriving show]
type dock = {
  component: componentCreator,
  width: option(int),
  height: option(int),
};

[@deriving show]
type splitTree =
  | Parent(direction, int, list(splitTree))
  | Leaf(split);

[@deriving show]
type splits = splitTree;

type t = {
  windows: splits,
  leftDock: option(dock),
  rightDock: option(dock),
};

let empty = Parent(Vertical, 0, []);

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

type parentHandler('a) = ('a, direction) => 'a;

let identity: parentHandler('a) = (result: 'a, _direction) => result;

let rec traverseSplitTree =
        (
          ~handleParent=identity,
          action,
          result,
          tree,
          direction: direction,
          (),
        ) =>
  switch (tree) {
  | Parent(direction, _, children) =>
    handleParent(result, direction)
    |> (
      newResult =>
        List.fold_left(
          (accum, child) =>
            traverseSplitTree(action, accum, child, direction, ()),
          newResult,
          children,
        )
    )
  | Leaf(split) => action(result, split, direction)
  };

let rec add = (id, split, tree) =>
  switch (tree) {
  | Parent(direction, parentId, children) when id == parentId =>
    Parent(direction, parentId, [Leaf(split), ...children])
  | Parent(direction, parentId, children) =>
    List.map(child => add(id, split, child), children)
    |> (newChildren => Parent(direction, parentId, newChildren))
  | Leaf(_) => tree
  };
