include AbstractTree;

[@deriving show({with_path: false})]
type metadata = {size: float};

[@deriving show({with_path: false})]
type t('id) = AbstractTree.node('id, metadata);

// DSL

module DSL = {
  open AbstractTree.DSL;

  let split = (~size=1., direction, children) =>
    split({size: size}, direction, children);
  let vsplit = (~size=1., children) => split(~size, `Vertical, children);
  let hsplit = (~size=1., children) => split(~size, `Horizontal, children);
  let window = (~size=1., id) => window({size: size}, id);

  let withSize = (size, node) => node |> withMetadata({size: size});
  let withChildren = withChildren;
};

include DSL;

let empty = vsplit([]);

let singleton = id => vsplit([window(id)]);

/**
 * addWindow
 */
let addWindow = (insertDirection, idToInsert, tree) => {
  switch (tree.kind) {
  | `Split(_, []) => window(~size=tree.meta.size, idToInsert)

  | `Split(direction, [firstChild, ...remainingChildren])
      when direction != insertDirection =>
    split(
      ~size=tree.meta.size,
      direction,
      [
        split(insertDirection, [window(idToInsert), firstChild]),
        ...remainingChildren,
      ],
    )

  | `Split(direction, children) =>
    split(
      ~size=tree.meta.size,
      direction,
      [window(idToInsert), ...children],
    )

  | `Window(id) =>
    split(
      ~size=tree.meta.size,
      insertDirection,
      [window(idToInsert), window(id)],
    )
  };
};

let%test_module "addWindow" =
  (module
   {
     let%test "add vertical split" = {
       let actual = empty |> addWindow(`Vertical, 1);

       actual == window(1);
     };

     let%test "parent split changes direction if needed" = {
       let actual =
         hsplit([window(2), window(1)]) |> addWindow(`Vertical, 3);

       actual == hsplit([vsplit([window(3), window(2)]), window(1)]);
     };
   });

/**
 * insertWindow
 */
let insertWindow = (position, insertDirection, idToInsert, tree) => {
  let `Before(targetId) | `After(targetId) = position;

  let splitWindow = node =>
    split(
      ~size=node.meta.size,
      insertDirection,
      switch (position) {
      | `Before(_) => [window(idToInsert), node |> withSize(1.)]
      | `After(_) => [node |> withSize(1.), window(idToInsert)]
      },
    );

  let rec traverse = node =>
    switch (node.kind) {
    | `Split(_, []) => {...node, kind: `Window(idToInsert)} // HACK: to work around this being intially called with an idea that doesn't yet exist in the tree
    | `Split(direction, children) when direction == insertDirection =>
      let onMatch = child =>
        switch (position) {
        | `Before(_) => [window(idToInsert), child]
        | `After(_) => [child, window(idToInsert)]
        };
      split(
        ~size=node.meta.size,
        direction,
        traverseChildren(~onMatch, [], children),
      );

    | `Split(direction, children) =>
      let onMatch = node => [splitWindow(node)];
      split(
        ~size=node.meta.size,
        direction,
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
        traverseChildren(~onMatch, List.rev(onMatch(child)) @ before, rest)

      | `Window(_) => traverseChildren(~onMatch, [child, ...before], rest)

      | `Split(_) =>
        traverseChildren(~onMatch, [traverse(child), ...before], rest)
      }
    };

  traverse(tree);
};

let%test_module "insertWindow" =
  (module
   {
     let%test "insert vertical split" = {
       let actual = window(1) |> insertWindow(`Before(1), `Vertical, 2);

       actual == vsplit([window(2), window(1)]);
     };

     let%test "insert vertical split - after" = {
       let actual = window(1) |> insertWindow(`After(1), `Vertical, 2);

       actual == vsplit([window(1), window(2)]);
     };

     let%test "parent split changes direction if needed" = {
       let actual =
         hsplit([window(2), window(1)])
         |> insertWindow(`Before(1), `Vertical, 3);

       actual == hsplit([window(2), vsplit([window(3), window(1)])]);
     };
   });

/**
 * removeWindow
 */
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

let%test_module "removeWindow" =
  (module
   {
     let%test "nested - remove 4" = {
       let initial =
         vsplit([
           hsplit([window(3), window(2)]),
           hsplit([window(4), window(1)]),
         ]);

       let actual = initial |> removeWindow(4);

       actual
       == vsplit([hsplit([window(3), window(2)]), hsplit([window(1)])]);
     };
     let%test "nested - remove 3" = {
       let initial =
         vsplit([hsplit([window(3), window(2)]), hsplit([window(1)])]);

       let actual = initial |> removeWindow(3);

       actual == vsplit([hsplit([window(2)]), hsplit([window(1)])]);
     };
     let%test "nested - remove 2 - empty parent split removed" = {
       let initial = vsplit([hsplit([window(2)]), hsplit([window(1)])]);

       let actual = initial |> removeWindow(2);

       actual == vsplit([hsplit([window(1)])]);
     };
   });

/**
 * resizeWindow
 */
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

let%test_module "resizeWindow" =
  (module
   {
     let%test "vsplit  - vresize" = {
       let initial = vsplit([window(1), window(2)]);

       let actual = resizeWindow(`Vertical, 2, 5., initial);

       actual == vsplit([window(1), window(2)]);
     };

     let%test "vsplit  - hresize" = {
       let initial = vsplit([window(1), window(2)]);

       let actual = resizeWindow(`Horizontal, 2, 5., initial);

       actual == vsplit([window(1), window(~size=5., 2)]);
     };

     let%test "hsplit  - hresize" = {
       let initial = hsplit([window(1), window(2)]);

       let actual = resizeWindow(`Horizontal, 2, 5., initial);

       actual == hsplit([window(1), window(2)]);
     };

     let%test "hsplit  - vresize" = {
       let initial = hsplit([window(1), window(2)]);

       let actual = resizeWindow(`Vertical, 2, 5., initial);

       actual == hsplit([window(1), window(~size=5., 2)]);
     };

     let%test "vsplit+hsplit - hresize" = {
       let initial = vsplit([window(1), hsplit([window(2), window(3)])]);

       let actual = resizeWindow(`Horizontal, 2, 5., initial);

       actual
       == vsplit([window(1), hsplit(~size=5., [window(2), window(3)])]);
     };

     let%test "vsplit+hsplit - vresize" = {
       let initial = vsplit([window(1), hsplit([window(2), window(3)])]);

       let actual = resizeWindow(`Vertical, 2, 5., initial);

       actual
       == vsplit([window(1), hsplit([window(~size=5., 2), window(3)])]);
     };

     let%test "hsplit+vsplit - hresize" = {
       let initial = hsplit([window(1), vsplit([window(2), window(3)])]);

       let actual = resizeWindow(`Horizontal, 2, 5., initial);

       actual
       == hsplit([window(1), vsplit([window(~size=5., 2), window(3)])]);
     };

     let%test "hsplit+vsplit - vresize" = {
       let initial = hsplit([window(1), vsplit([window(2), window(3)])]);

       let actual = resizeWindow(`Vertical, 2, 5., initial);

       actual
       == hsplit([window(1), vsplit(~size=5., [window(2), window(3)])]);
     };
   });

/**
 * resizeSplit
 */
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

/**
 * resetWeights
 */
let resetWeights = tree => AbstractTree.map(withSize(1.), tree);
