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

// INTERNAL
open {
       let totalWeight = nodes =>
         nodes
         |> List.map(child => child.meta.size)
         |> List.fold_left((+.), 0.)
         |> max(1.);

       /**
        * shiftWeightRight
        *
        * Shifts weight from `nodes[index]` to `nodes[index+1]` if delta is positive,
        * vice versa if negative.
        */
       let shiftWeightRight = (~delta, index, nodes) => {
         let totalWeight = totalWeight(nodes);
         let minimumWeight =
           min(
             0.1 *. totalWeight,
             totalWeight /. float(List.length(nodes)),
           );

         let rec loop = i =>
           fun
           | [] => []
           | [node] => [node]
           | [node, next, ...rest] when i == index => {
               let delta =
                 if (node.meta.size +. delta < minimumWeight) {
                   min(0., -. (node.meta.size -. minimumWeight));
                 } else if (next.meta.size +. delta < minimumWeight) {
                   min(0., -. (next.meta.size -. minimumWeight));
                 } else {
                   delta;
                 };

               [
                 node |> withSize(node.meta.size +. delta),
                 next |> withSize(next.meta.size -. delta),
                 ...rest,
               ];
             }
           | [node, ...rest] => [node, ...loop(i + 1, rest)];

         loop(0, nodes);
       };

       let%test_module "shiftWeightRight" =
         (module
          {
            let sizes = List.map(node => node.meta.size);

            let%test "sanity check: sizes - even" =
              sizes([window(1), window(2), window(3)]) == [1., 1., 1.];
            let%test "sanity check: sizes - uneven" =
              sizes([
                window(~size=0.95, 1),
                window(~size=1.05, 2),
                window(3),
              ])
              == [0.95, 1.05, 1.];

            let%test "positive delta" = {
              let initial = [window(1), window(2), window(3)];

              let actual = initial |> shiftWeightRight(~delta=0.05, 1);

              sizes(actual) == [1., 1.05, 0.95];
            };

            let%test "negative delta" = {
              let initial = [window(1), window(2), window(3)];

              let actual = initial |> shiftWeightRight(~delta=-0.05, 1);

              sizes(actual) == [1., 0.95, 1.05];
            };

            let%test "target below minimum" = {
              let initial = [window(1), window(~size=0.1, 2), window(3)];

              let actual = initial |> shiftWeightRight(~delta=0.05, 1);

              sizes(actual) == sizes(initial);
            };

            let%test "next below minimum - negative delta" = {
              let initial = [window(1), window(2), window(~size=0.1, 3)];

              let actual = initial |> shiftWeightRight(~delta=-0.05, 1);

              sizes(actual) == sizes(initial);
            };
          });
     };

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
  let splitWindow = node =>
    split(
      ~size=node.meta.size,
      insertDirection,
      switch (position) {
      | `Before(_) => [window(idToInsert), node |> withSize(1.)]
      | `After(_) => [node |> withSize(1.), window(idToInsert)]
      },
    );

  let replace = (index, newNode) =>
    List.mapi((i, node) => i == index ? newNode : node);

  let insertBefore = (i, node, nodes) => {
    let left = Base.List.take(nodes, i - 1);
    let right = Base.List.drop(nodes, i - 1);
    left @ [node] @ right;
  };

  let insertAfter = (i, node, nodes) => {
    let left = Base.List.take(nodes, i);
    let right = Base.List.drop(nodes, i);
    left @ [node] @ right;
  };

  let rec traverse = (path, node) =>
    switch (path, node.kind) {
    | ([], _) => splitWindow(node)

    | ([i], `Split(direction, children)) when direction == insertDirection =>
      let children =
        switch (position) {
        | `Before(_) => insertBefore(i, window(idToInsert), children)
        | `After(_) => insertAfter(i, window(idToInsert), children)
        };
      node |> withChildren(children);

    | ([i, ...rest], `Split(_, children)) =>
      switch (List.nth_opt(children, i)) {
      | Some(child) =>
        let children = replace(i, traverse(rest, child), children);
        node |> withChildren(children);
      | None => node
      }

    // shouldn't happen
    | (_, `Window(_)) => node
    };

  let `Before(targetId) | `After(targetId) = position;
  switch (AbstractTree.path(targetId, tree)) {
  | Some(path) => traverse(path, tree)
  | None => tree
  };
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
  | [] => node // shouldn't happen

  | [index] =>
    switch (node.kind) {
    | `Split(direction, children) =>
      split(
        ~size=node.meta.size,
        direction,
        shiftWeightRight(
          ~delta=totalWeight(children) *. delta,
          index,
          children,
        ),
      )

    | `Window(_) => node
    }

  | [index, ...rest] =>
    switch (node.kind) {
    | `Split(direction, children) =>
      split(
        ~size=node.meta.size,
        direction,
        List.mapi(
          (i, child) => {
            i == index ? resizeSplit(~path=rest, ~delta, child) : child
          },
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
