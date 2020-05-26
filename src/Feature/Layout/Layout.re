include AbstractTree;

type metadata = {size: float};

type t('id) = AbstractTree.node('id, metadata);

module DSL = {
  let split = (~size=1., direction, children) => {
    meta: {
      size: size,
    },
    kind: `Split((direction, children)),
  };
  let vertical = (~size=1., children) => split(~size, `Vertical, children);
  let horizontal = (~size=1., children) =>
    split(~size, `Horizontal, children);
  let window = (~size=1., id) => {
    meta: {
      size: size,
    },
    kind: `Window(id),
  };

  let withSize = (size, node) => {
    ...node,
    meta: {
      size: size,
    },
  };
};

include DSL;

let empty = vertical([]);

let singleton = id => vertical([window(id)]);

let addWindow = (~target=None, ~position, direction, idToInsert, tree) => {
  let splitWindow = node =>
    split(
      ~size=node.meta.size,
      direction,
      switch (position) {
      | `Before => [window(idToInsert), node |> withSize(1.)]
      | `After => [node |> withSize(1.), window(idToInsert)]
      },
    );

  switch (target) {
  | Some(targetId) =>
    let rec traverse = node =>
      switch (node.kind) {
      | `Split(_, []) => {...node, kind: `Window(idToInsert)} // HACK: to work around this being intially called with an idea that doesn't yet exist in the tree
      | `Split(thisDirection, children) when thisDirection == direction =>
        let onMatch = child =>
          switch (position) {
          | `Before => [window(idToInsert), child]
          | `After => [child, window(idToInsert)]
          };
        split(
          ~size=node.meta.size,
          thisDirection,
          traverseChildren(~onMatch, [], children),
        );

      | `Split(thisDirection, children) =>
        let onMatch = node => [splitWindow(node)];
        split(
          ~size=node.meta.size,
          thisDirection,
          traverseChildren(~onMatch, [], children),
        );

      | `Window(id) when id == targetId => splitWindow(node)

      | `Window(_) => node
      }

    and traverseChildren = (~onMatch, before, after) =>
      switch (after) {
      | [] => List.rev(before)
      | [child, ...rest] =>
        switch (child.kind) {
        | `Window(id) when id == targetId =>
          traverseChildren(
            ~onMatch,
            List.rev(onMatch(child)) @ before,
            rest,
          )

        | `Window(_) => traverseChildren(~onMatch, [child, ...before], rest)

        | `Split(_) =>
          traverseChildren(~onMatch, [traverse(child), ...before], rest)
        }
      };

    traverse(tree);

  | None =>
    switch (tree.kind) {
    | `Split(_, []) => window(~size=tree.meta.size, idToInsert)

    | `Split(direction, children) =>
      split(
        ~size=tree.meta.size,
        direction,
        [window(idToInsert), ...children],
      )

    | `Window(id) =>
      split(
        ~size=tree.meta.size,
        direction,
        [window(idToInsert), window(id)],
      )
    }
  };
};

let removeWindow = (target, tree) => {
  let rec traverse = node =>
    switch (node.kind) {
    | `Split(direction, children) =>
      switch (List.filter_map(traverse, children)) {
      | [] => None
      // BUG: Collapsing disabled as it doesn't preserve size properly.
      // | [child] => Some(child)
      | newChildren =>
        Some(split(~size=node.meta.size, direction, newChildren))
      }
    | `Window(id) when id == target => None
    | `Window(_) => Some(node)
    };

  traverse(tree) |> Option.value(~default=empty);
};

let resizeWindow = (direction, targetId, factor, node) => {
  let rec traverse = (~parentDirection=?, node) =>
    switch (node.kind) {
    | `Split(dir, children) =>
      let (result, children) =
        List.fold_left(
          ((accResult, accChildren), child) => {
            let (result, newChild) = traverse(~parentDirection=dir, child);

            (
              result == `NotFound ? accResult : result,
              [newChild, ...accChildren],
            );
          },
          (`NotFound, []),
          List.rev(children),
        );

      switch (result, parentDirection) {
      | (`NotAdjusted, Some(parentDirection))
          when parentDirection != direction => (
          `Adjusted,
          split(~size=node.meta.size *. factor, dir, children),
        )

      | _ => (result, split(~size=node.meta.size, dir, children))
      };

    | `Window(id) when id == targetId =>
      if (parentDirection == Some(direction)) {
        (`NotAdjusted, node);
      } else {
        (`Adjusted, node |> withSize(node.meta.size *. factor));
      }

    | `Window(_) => (`NotFound, node)
    };

  traverse(node) |> snd;
};

let rec resizeSplit = (~path, ~delta, node) => {
  switch (path) {
  | [] => node
  | [index] =>
    switch (node.kind) {
    | `Split(direction, children) =>
      let childCount = List.length(children);
      let totalWeight =
        children
        |> List.map(child => child.meta.size)
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
            let weight = node.meta.size;
            let nextWeight = next.meta.size;
            let deltaWeight =
              if (weight +. deltaWeight < minimumWeight) {
                -. (weight -. minimumWeight);
              } else if (nextWeight -. deltaWeight < minimumWeight) {
                nextWeight -. minimumWeight;
              } else {
                deltaWeight;
              };

            [
              node |> withSize(weight +. deltaWeight),
              next |> withSize(nextWeight -. deltaWeight),
              ...rest,
            ];
          }
        | [node, ...rest] => [node, ...resizeChildren(i + 1, rest)]
      );

      split(~size=node.meta.size, direction, resizeChildren(0, children));

    | `Window(_) => node
    }

  | [index, ...rest] =>
    switch (node.kind) {
    | `Split(direction, children) =>
      split(
        ~size=node.meta.size,
        direction,
        List.mapi(
          (i, child) =>
            i == index ? resizeSplit(~path=rest, ~delta, child) : child,
          children,
        ),
      )
    | `Window(_) => node
    }
  };
};

let resetWeights = tree => AbstractTree.map(withSize(1.), tree);
