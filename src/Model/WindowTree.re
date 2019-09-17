open Oni_Core;

module WindowSplitId =
  Revery.UniqueId.Make({});

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

let empty = Parent(Vertical, [Empty]);

let createSplit = (~width=?, ~height=?, ~editorGroupId, ()) => {
  id: WindowSplitId.getUniqueId(),
  editorGroupId,
  width,
  height,
};

let getSplits = (tree: t) => {
  let rec f = (tree: t, splits: list(split)) => {
    switch (tree) {
    | Parent(_, children) =>
      List.fold_left((c, curr) => f(curr, c), splits, children)
    | Leaf(split) => [split, ...splits]
    | Empty => splits
    };
  };

  f(tree, []);
};

let filterEmpty = v =>
  switch (v) {
  | Empty => false
  | _ => true
  };

let rec getEditorGroupIdFromSplitId = (splitId: int, currentTree) => {
  switch (currentTree) {
  | Parent(_, tree) =>
    List.fold_left(
      (prev, curr) =>
        switch (prev) {
        | Some(_) => prev
        | None => getEditorGroupIdFromSplitId(splitId, curr)
        },
      None,
      tree,
    )
  | Leaf(split) => split.id == splitId ? Some(split.editorGroupId) : None
  | Empty => None
  };
};

let addSplit = (~target=None, direction, newSplit, currentTree) => {
  let rec f = (targetId, parent, split) => {
    switch (split) {
    | Parent(d, tree) => [
        Parent(d, List.concat(List.map(f(targetId, Some(split)), tree))),
      ]
    | Leaf(v) =>
      if (v.id == targetId) {
        switch (parent) {
        | Some(Parent(dir, _)) =>
          if (dir == direction) {
            [Leaf(newSplit), Leaf(v)];
          } else {
            [Parent(direction, [Leaf(newSplit), Leaf(v)])];
          }
        | _ => [Leaf(newSplit), Leaf(v)]
        };
      } else {
        [Leaf(v)];
      }
    | Empty => [Leaf(newSplit)]
    };
  };

  switch (target) {
  | Some(targetId) => f(targetId, None, currentTree) |> List.hd
  | None =>
    switch (currentTree) {
    | Parent(d, tree) =>
      Parent(d, List.filter(filterEmpty, [Leaf(newSplit), ...tree]))
    | v => v
    }
  };
};

let rec removeSplit = (id, currentTree) =>
  switch (currentTree) {
  | Parent(direction, children) =>
    let newChildren =
      children
      |> List.map(child => removeSplit(id, child))
      |> List.filter(filterEmpty);
    Parent(direction, newChildren);
  | Leaf(split) when split.id == id => Empty
  | Leaf(_) as leaf => leaf
  | Empty => Empty
  };

let rec rotate = (id, func, currenTree) => {
  let findSplit = (id, children) => {
    let predicate =
      fun
      | Leaf(split) => split.id == id
      | _ => false;

    List.exists(predicate, children);
  };

  switch (currenTree) {
  | Parent(direction, children) =>
    Parent(
      direction,
      List.map(
        rotate(id, func),
        findSplit(id, children) ? func(children) : children,
      ),
    )
  | Leaf(_) as leaf => leaf
  | Empty => Empty
  };
};

let rotateForward = (id, currentTree) => {
  let f =
    fun
    | [] => []
    | [a] => [a]
    | [a, b] => [b, a]
    | l =>
      switch (Utility.last(l)) {
      | Some(x) => [x, ...Utility.dropLast(l)]
      | None => []
      };

  rotate(id, f, currentTree);
};

let rotateBackward = (id, currentTree) => {
  let f =
    fun
    | [] => []
    | [a] => [a]
    | [a, b] => [b, a]
    | [head, ...tail] => tail @ [head];

  rotate(id, f, currentTree);
};
