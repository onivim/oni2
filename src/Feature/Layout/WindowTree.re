open Oni_Core;
open Utility;

[@deriving show({with_path: false})]
type direction =
  | Horizontal
  | Vertical;

type t =
  | Split([ | `Horizontal | `Vertical], list(t))
  | Window({
      weight: float,
      content: int,
    })
  | Empty;

let empty = Split(`Vertical, [Empty]);

let getSplits = tree => {
  let rec traverse = (node, acc) => {
    switch (node) {
    | Split(_, children) =>
      List.fold_left((acc, child) => traverse(child, acc), acc, children)
    | Window({content, _}) => [content, ...acc]
    | Empty => acc
    };
  };

  traverse(tree, []);
};

let addSplit = (~target=None, ~position, direction, content, tree) => {
  let newWindow = Window({weight: 1., content});

  let rec f = (targetId, parent, split) => {
    switch (split) {
    | Split(direction, children) => [
        Split(
          direction,
          List.concat(List.map(f(targetId, Some(split)), children)),
        ),
      ]
    | Window({content, _}) as window =>
      if (content == targetId) {
        let children =
          switch (position) {
          | `Before => [newWindow, window]
          | `After => [window, newWindow]
          };

        switch (parent) {
        | Some(Split(dir, _)) =>
          if (dir == direction) {
            children;
          } else {
            [Split(direction, children)];
          }
        | _ => children
        };
      } else {
        [window];
      }
    | Empty => [newWindow]
    };
  };

  switch (target) {
  | Some(targetId) => f(targetId, None, tree) |> List.hd
  | None =>
    switch (tree) {
    | Split(d, children) =>
      Split(d, List.filter(node => node != Empty, [newWindow, ...children]))
    | other => other
    }
  };
};

let rec removeSplit = (target, tree) =>
  switch (tree) {
  | Split(direction, children) =>
    let newChildren =
      children
      |> List.map(child => removeSplit(target, child))
      |> List.filter(node => node != Empty);

    if (List.length(newChildren) > 0) {
      Split(direction, newChildren);
    } else {
      Empty;
    };
  | Window({content, _}) when content == target => Empty
  | Window(_) as window => window
  | Empty => Empty
  };

let rec rotate = (target, func, currenTree) => {
  let findSplit = children => {
    let predicate =
      fun
      | Window({content, _}) => content == target
      | _ => false;

    List.exists(predicate, children);
  };

  switch (currenTree) {
  | Split(direction, children) =>
    Split(
      direction,
      List.map(
        rotate(target, func),
        findSplit(children) ? func(children) : children,
      ),
    )
  | Window(_) as window => window
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
