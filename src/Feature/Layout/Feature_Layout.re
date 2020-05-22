open Utility;

[@deriving show({with_path: false})]
type size =
  | Weight(float);

[@deriving show({with_path: false})]
type t('id) =
  | Split([ | `Horizontal | `Vertical], size, list(t('id)))
  | Window(size, 'id);

let nodeSize =
  fun
  | Split(_, size, _) => size
  | Window(size, _) => size;

let withSize = size =>
  fun
  | Split(direction, _, children) => Split(direction, size, children)
  | Window(_, id) => Window(size, id);

let nodeWeight =
  fun
  | Split(_, Weight(weight), _) => Some(weight)
  | Window(Weight(weight), _) => Some(weight);

[@deriving show({with_path: false})]
type sized('id) = {
  x: int,
  y: int,
  width: int,
  height: int,
  kind: [
    | `Split([ | `Horizontal | `Vertical], list(sized('id)))
    | `Window('id)
  ],
};

module Internal = {
  let contains = (x, y, split) => {
    x >= split.x
    && x <= split.x
    + split.width
    && y >= split.y
    && y <= split.y
    + split.height;
  };

  let rec sizedWindows = node =>
    switch (node) {
    | {kind: `Window(_), _} => [node]
    | {kind: `Split(_, children), _} =>
      children |> List.map(sizedWindows) |> List.concat
    };

  let move = (targetId, dirX, dirY, node) => {
    let splits = sizedWindows(node);

    let (minX, minY, maxX, maxY, deltaX, deltaY) =
      List.fold_left(
        (prev, cur) => {
          let (minX, minY, maxX, maxY, deltaX, deltaY) = prev;

          let newMinX = cur.x < minX ? cur.x : minX;
          let newMinY = cur.y < minY ? cur.y : minY;
          let newMaxX = cur.x + cur.width > maxX ? cur.x + cur.width : maxX;
          let newMaxY = cur.y + cur.height > maxY ? cur.y + cur.height : maxY;
          let newDeltaX = cur.width / 2 < deltaX ? cur.width / 2 : deltaX;
          let newDeltaY = cur.height / 2 < deltaY ? cur.height / 2 : deltaY;

          (newMinX, newMinY, newMaxX, newMaxY, newDeltaX, newDeltaY);
        },
        (0, 0, 1, 1, 100, 100),
        splits,
      );

    switch (List.find_opt(split => split.kind == `Window(targetId), splits)) {
    | None => None
    | Some(target) =>
      let curX = ref(target.x + target.width / 2);
      let curY = ref(target.y + target.height / 2);
      let found = ref(false);
      let result = ref(None);

      while (! found^
             && curX^ >= minX
             && curX^ < maxX
             && curY^ >= minY
             && curY^ < maxY) {
        let x = curX^;
        let y = curY^;

        switch (
          List.find_opt(
            s => s.kind != `Window(targetId) && contains(x, y, s),
            splits,
          )
        ) {
        | Some({kind: `Window(id), _}) =>
          result := Some(id);
          found := true;
        | _ => ()
        };

        curX := x + dirX * deltaX;
        curY := y + dirY * deltaY;
      };

      result^;
    };
  };

  let rec rotate = (target, func, tree) => {
    let findSplit = children => {
      let predicate =
        fun
        | Window(_, id) => id == target
        | _ => false;

      List.exists(predicate, children);
    };

    switch (tree) {
    | Split(direction, size, children) =>
      Split(
        direction,
        size,
        List.map(
          child => rotate(target, func, child),
          findSplit(children) ? func(children) : children,
        ),
      )
    | Window(_) as window => window
    };
  };
};

let empty = Split(`Vertical, Weight(1.), []);
let initial = empty;

let windows = tree => {
  let rec traverse = (node, acc) => {
    switch (node) {
    | Split(_, _, children) =>
      List.fold_left((acc, child) => traverse(child, acc), acc, children)
    | Window(_, id) => [id, ...acc]
    };
  };

  traverse(tree, []);
};

let rec layout = (x, y, width, height, tree) => {
  switch (tree) {
  | Split(direction, _, children) =>
    let totalWeight =
      children
      |> List.filter_map(nodeWeight)
      |> List.fold_left((+.), 0.)
      |> max(1.);

    let sizedChildren =
      (
        switch (direction) {
        | `Horizontal =>
          let unitHeight = float(height) /. totalWeight;
          List.fold_left(
            ((y, acc), child) => {
              switch (nodeSize(child)) {
              | Weight(weight) =>
                let height = int_of_float(unitHeight *. weight);
                let sized = layout(x, y, width, height, child);
                (y + height, [sized, ...acc]);
              }
            },
            (y, []),
            children,
          );

        | `Vertical =>
          let unitWidth = float(width) /. totalWeight;
          List.fold_left(
            ((x, acc), child) => {
              switch (nodeSize(child)) {
              | Weight(weight) =>
                let width = int_of_float(unitWidth *. weight);
                let sized = layout(x, y, width, height, child);
                (x + width, [sized, ...acc]);
              }
            },
            (x, []),
            children,
          );
        }
      )
      |> snd
      |> List.rev;

    {x, y, width, height, kind: `Split((direction, sizedChildren))};

  | Window(_, id) => {x, y, width, height, kind: `Window(id)}
  };
};

let addWindow = (~target=None, ~position, direction, id, tree) => {
  let newWindow = Window(Weight(1.), id);
  switch (target) {
  | Some(targetId) =>
    let rec traverse = node => {
      switch (node) {
      | Split(_, size, []) => Window(size, id) // HACK: to work around this being intially called with an idea that doesn't yet exist in the tree
      | Split(thisDirection, size, children) when thisDirection == direction =>
        let onMatch = child =>
          switch (position) {
          | `Before => [newWindow, child]
          | `After => [child, newWindow]
          };
        Split(thisDirection, size, traverseChildren(~onMatch, [], children));

      | Split(thisDirection, size, children) =>
        let onMatch = child =>
          switch (position) {
          | `Before => [
              Split(
                direction,
                nodeSize(child),
                [newWindow, child |> withSize(Weight(1.))],
              ),
            ]
          | `After => [
              Split(
                direction,
                nodeSize(child),
                [child |> withSize(Weight(1.)), newWindow],
              ),
            ]
          };
        Split(thisDirection, size, traverseChildren(~onMatch, [], children));

      | Window(size, id) when id == targetId =>
        switch (position) {
        | `Before =>
          Split(direction, size, [newWindow, Window(Weight(1.), id)])
        | `After =>
          Split(direction, size, [Window(Weight(1.), id), newWindow])
        }

      | Window(_) as window => window
      };
    }

    and traverseChildren = (~onMatch, before, after) =>
      switch (after) {
      | [] => List.rev(before)
      | [head, ...rest] =>
        switch (head) {
        | Window(_, id) as child when id == targetId =>
          traverseChildren(
            ~onMatch,
            List.rev(onMatch(child)) @ before,
            rest,
          )

        | Split(_) as child =>
          traverseChildren(~onMatch, [traverse(child), ...before], rest)

        | child => traverseChildren(~onMatch, [child, ...before], rest)
        }
      };

    traverse(tree);

  | None =>
    switch (tree) {
    | Split(_, size, []) => Window(size, id)
    | Split(d, size, children) => Split(d, size, [newWindow, ...children])
    | Window(size, id) =>
      Split(direction, size, [newWindow, Window(Weight(1.), id)])
    }
  };
};

let removeWindow = (target, tree) => {
  let rec traverse =
    fun
    | Split(direction, size, children) =>
      switch (List.filter_map(traverse, children)) {
      | [] => None
      // BUG: Collapsing disabled as it doesn't preserve size properly.
      // | [child] => Some(child)
      | newChildren => Some(Split(direction, size, newChildren))
      }
    | Window(_, id) when id == target => None
    | node => Some(node);

  traverse(tree) |> Option.value(~default=empty);
};

let move = (current, dirX, dirY, tree) => {
  let layout = layout(0, 0, 200, 200, tree);

  Internal.move(current, dirX, dirY, layout)
  |> Option.value(~default=current);
};

let moveLeft = current => move(current, -1, 0);
let moveRight = current => move(current, 1, 0);
let moveUp = current => move(current, 0, -1);
let moveDown = current => move(current, 0, 1);

let rotateForward = (target, tree) => {
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

  Internal.rotate(target, f, tree);
};

let rotateBackward = (target, tree) => {
  let f =
    fun
    | [] => []
    | [a] => [a]
    | [a, b] => [b, a]
    | [head, ...tail] => tail @ [head];

  Internal.rotate(target, f, tree);
};

let resizeWindow = (direction, target, factor, node) => {
  let rec traverse = (~parentDirection=?) =>
    fun
    | Split(dir, Weight(weight) as size, children) => {
        let (result, children) =
          List.fold_left(
            ((accResult, accChildren), child) => {
              let (result, node) = traverse(~parentDirection=dir, child);
              (
                result == `NotFound ? accResult : result,
                [node, ...accChildren],
              );
            },
            (`NotFound, []),
            List.rev(children),
          );

        switch (result, parentDirection) {
        | (`NotAdjusted, Some(parentDirection))
            when parentDirection != direction => (
            `Adjusted,
            Split(dir, Weight(weight *. factor), children),
          )

        | _ => (result, Split(dir, size, children))
        };
      }

    | Window(Weight(weight), id) as window when id == target =>
      if (parentDirection == Some(direction)) {
        (`NotAdjusted, window);
      } else {
        (`Adjusted, Window(Weight(weight *. factor), id));
      }

    | Window(_) as window => (`NotFound, window);

  traverse(node) |> snd;
};

let rec resizeSplit = (~path, ~delta, model) => {
  switch (path) {
  | [] => model
  | [index] =>
    switch (model) {
    | Split(direction, size, children) =>
      let childCount = List.length(children);
      let totalWeight =
        children
        |> List.filter_map(nodeWeight)
        |> List.fold_left((+.), 0.)
        |> max(1.);
      let minimumWeight =
        min(0.1 *. totalWeight, totalWeight /. float(childCount));
      let deltaWeight = totalWeight *. delta;

      let rec resizeChildren = i => (
        fun
        | [] => [] // shouldn't happen
        | [node] => [node] // shouldn't happen
        | [node, next, ...rest] when index == i => {
            let weight = Option.get(nodeWeight(node));
            let nextWeight = Option.get(nodeWeight(next));
            let deltaWeight =
              if (weight +. deltaWeight < minimumWeight) {
                -. (weight -. minimumWeight);
              } else if (nextWeight -. deltaWeight < minimumWeight) {
                nextWeight -. minimumWeight;
              } else {
                deltaWeight;
              };

            [
              node |> withSize(Weight(weight +. deltaWeight)),
              next |> withSize(Weight(nextWeight -. deltaWeight)),
              ...rest,
            ];
          }
        | [node, ...rest] => [node, ...resizeChildren(i + 1, rest)]
      );

      Split(direction, size, resizeChildren(0, children));

    | Window(_) => model
    }
  | [index, ...rest] =>
    switch (model) {
    | Split(direction, size, children) =>
      Split(
        direction,
        size,
        List.mapi(
          (i, child) =>
            i == index ? resizeSplit(~path=rest, ~delta, child) : child,
          children,
        ),
      )
    | Window(_) => model
    }
  };
};

let rec resetWeights =
  fun
  | Split(direction, Weight(_), children) =>
    Split(direction, Weight(1.), List.map(resetWeights, children))
  | Window(_, id) => Window(Weight(1.), id);
