
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
type t =
  | Parent(direction, list(t))
  | Leaf(split)
  | Empty;

let initialWindowId = WindowId.next();

let empty = Parent(Vertical, initialWindowId, []);

let createSplit = (~width=?, ~height=?, ~editorGroupId, ()) => {
  id: WindowSplitId.getUniqueId(),
  editorGroupId,
  width,
  height,
};

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
