open Oni_Core;
open Utility;

module WindowSplitId =
  Revery.UniqueId.Make({});

[@deriving show({with_path: false})]
type direction =
  | Horizontal
  | Vertical;

type position =
  | Before
  | After;

[@deriving show({with_path: false})]
type split = {
  id: int,
  editorGroupId: int,
  width: option(int),
  height: option(int),
};

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

let addSplit = (~target=None, ~position, direction, newSplit, currentTree) => {
  let rec f = (targetId, parent, split) => {
    switch (split) {
    | Parent(d, tree) => [
        Parent(d, List.concat(List.map(f(targetId, Some(split)), tree))),
      ]
    | Leaf(v) =>
      if (v.editorGroupId == targetId) {
        let children =
          switch (position) {
          | Before => [Leaf(newSplit), Leaf(v)]
          | After => [Leaf(v), Leaf(newSplit)]
          };
        switch (parent) {
        | Some(Parent(dir, _)) =>
          if (dir == direction) {
            children;
          } else {
            [Parent(direction, children)];
          }
        | _ => children
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

let rec removeSplit = (content, currentTree) =>
  switch (currentTree) {
  | Parent(direction, children) =>
    let newChildren =
      children
      |> List.map(child => removeSplit(content, child))
      |> List.filter(filterEmpty);

    if (List.length(newChildren) > 0) {
      Parent(direction, newChildren);
    } else {
      Empty;
    };
  | Leaf(split) when split.editorGroupId == content => Empty
  | Leaf(_) as leaf => leaf
  | Empty => Empty
  };

let rec rotate = (target, func, currenTree) => {
  let findSplit = children => {
    let predicate =
      fun
      | Leaf(split) => split.editorGroupId == target
      | _ => false;

    List.exists(predicate, children);
  };

  switch (currenTree) {
  | Parent(direction, children) =>
    Parent(
      direction,
      List.map(
        rotate(target, func),
        findSplit(children) ? func(children) : children,
      ),
    )
  | Leaf(_) as leaf => leaf
  | Empty => Empty
  };
};

let rotateForward = (target, currentTree) => {
  let f =
    fun
    | [] => []
    | [a] => [a]
    | [a, b] => [b, a]
    | list =>
      switch (ListEx.last(list)) {
      | Some(x) => [x, ...ListEx.dropLast(list)]
      | None => []
      };

  rotate(target, f, currentTree);
};

let rotateBackward = (target, currentTree) => {
  let f =
    fun
    | [] => []
    | [a] => [a]
    | [a, b] => [b, a]
    | [head, ...tail] => tail @ [head];

  rotate(target, f, currentTree);
};

let rec windowFor = (content, node) => {
  let rec loopChildren =
    fun
    | [] => None
    | [head, ...rest] =>
      switch (windowFor(content, head)) {
      | Some(split) => Some(split)
      | None => loopChildren(rest)
      };

  switch (node) {
  | Parent(_, children) => loopChildren(children)
  | Leaf(split) when split.editorGroupId == content => Some(split)
  | Leaf(_)
  | Empty => None
  };
};
