include AbstractTree;

[@deriving show({with_path: false})]
type metadata = {weight: float};

[@deriving show({with_path: false})]
type t('id) = AbstractTree.node('id, metadata);

// DSL

module DSL = {
  open AbstractTree.DSL;

  let split = (~weight=1., direction, children) =>
    split({weight: weight}, direction, children);
  let vsplit = (~weight=1., children) => split(~weight, `Vertical, children);
  let hsplit = (~weight=1., children) =>
    split(~weight, `Horizontal, children);
  let window = (~weight=1., id) => window({weight: weight}, id);

  let withWeight = (weight, node) => node |> withMetadata({weight: weight});
  let withChildren = withChildren;
};

include DSL;

let empty = vsplit([]);

let singleton = id => vsplit([window(id)]);

// INTERNAL
open {
       let totalWeight = nodes =>
         nodes
         |> List.map(child => child.meta.weight)
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
                 if (node.meta.weight +. delta < minimumWeight) {
                   min(0., -. (node.meta.weight -. minimumWeight));
                 } else if (next.meta.weight -. delta < minimumWeight) {
                   max(0., next.meta.weight -. minimumWeight);
                 } else {
                   delta;
                 };

               [
                 node |> withWeight(node.meta.weight +. delta),
                 next |> withWeight(next.meta.weight -. delta),
                 ...rest,
               ];
             }
           | [node, ...rest] => [node, ...loop(i + 1, rest)];

         loop(0, nodes);
       };

       let%test_module "shiftWeightRight" =
         (module
          {
            let weights = List.map(node => node.meta.weight);

            let%test "sanity check: weights - even" =
              weights([window(1), window(2), window(3)]) == [1., 1., 1.];
            let%test "sanity check: weights - uneven" =
              weights([
                window(~weight=0.95, 1),
                window(~weight=1.05, 2),
                window(3),
              ])
              == [0.95, 1.05, 1.];

            let%test "positive delta" = {
              let initial = [window(1), window(2), window(3)];

              let actual = initial |> shiftWeightRight(~delta=0.05, 1);

              weights(actual) == [1., 1.05, 0.95];
            };

            let%test "negative delta" = {
              let initial = [window(1), window(2), window(3)];

              let actual = initial |> shiftWeightRight(~delta=-0.05, 1);

              weights(actual) == [1., 0.95, 1.05];
            };

            let%test "target below minimum" = {
              let initial = [window(1), window(~weight=0.1, 2), window(3)];

              let actual = initial |> shiftWeightRight(~delta=0.05, 1);

              weights(actual) == weights(initial);
            };

            let%test "next below minimum - negative delta" = {
              let initial = [window(1), window(2), window(~weight=0.1, 3)];

              let actual = initial |> shiftWeightRight(~delta=-0.05, 1);

              weights(actual) == weights(initial);
            };
          });
     };

/**
 * addWindow
 */
let addWindow = (insertDirection, idToInsert, tree) => {
  switch (tree.kind) {
  | `Split(_, []) => window(~weight=tree.meta.weight, idToInsert)

  | `Split(direction, [firstChild, ...remainingChildren])
      when direction != insertDirection =>
    split(
      ~weight=tree.meta.weight,
      direction,
      [
        split(insertDirection, [window(idToInsert), firstChild]),
        ...remainingChildren,
      ],
    )

  | `Split(direction, children) =>
    split(
      ~weight=tree.meta.weight,
      direction,
      [window(idToInsert), ...children],
    )

  | `Window(id) =>
    split(
      ~weight=tree.meta.weight,
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
      ~weight=node.meta.weight,
      insertDirection,
      switch (position) {
      | `Before(_) => [window(idToInsert), node |> withWeight(1.)]
      | `After(_) => [node |> withWeight(1.), window(idToInsert)]
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
      // BUG: Collapsing disabled as it doesn't preserve weight properly.
      // | [child] => Some(child)
      | newChildren =>
        Some(split(~weight=node.meta.weight, direction, newChildren))
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
          split(~weight=node.meta.weight *. factor, dir, children),
        )

      | _ => (result, split(~weight=node.meta.weight, dir, children))
      };

    | `Window(id) when id == targetId =>
      if (parentDirection == Some(direction)) {
        (`NotAdjusted, node);
      } else {
        (`Adjusted, node |> withWeight(node.meta.weight *. factor));
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

       actual == vsplit([window(1), window(~weight=5., 2)]);
     };

     let%test "hsplit  - hresize" = {
       let initial = hsplit([window(1), window(2)]);

       let actual = resizeWindow(`Horizontal, 2, 5., initial);

       actual == hsplit([window(1), window(2)]);
     };

     let%test "hsplit  - vresize" = {
       let initial = hsplit([window(1), window(2)]);

       let actual = resizeWindow(`Vertical, 2, 5., initial);

       actual == hsplit([window(1), window(~weight=5., 2)]);
     };

     let%test "vsplit+hsplit - hresize" = {
       let initial = vsplit([window(1), hsplit([window(2), window(3)])]);

       let actual = resizeWindow(`Horizontal, 2, 5., initial);

       actual
       == vsplit([window(1), hsplit(~weight=5., [window(2), window(3)])]);
     };

     let%test "vsplit+hsplit - vresize" = {
       let initial = vsplit([window(1), hsplit([window(2), window(3)])]);

       let actual = resizeWindow(`Vertical, 2, 5., initial);

       actual
       == vsplit([window(1), hsplit([window(~weight=5., 2), window(3)])]);
     };

     let%test "hsplit+vsplit - hresize" = {
       let initial = hsplit([window(1), vsplit([window(2), window(3)])]);

       let actual = resizeWindow(`Horizontal, 2, 5., initial);

       actual
       == hsplit([window(1), vsplit([window(~weight=5., 2), window(3)])]);
     };

     let%test "hsplit+vsplit - vresize" = {
       let initial = hsplit([window(1), vsplit([window(2), window(3)])]);

       let actual = resizeWindow(`Vertical, 2, 5., initial);

       actual
       == hsplit([window(1), vsplit(~weight=5., [window(2), window(3)])]);
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
        ~weight=node.meta.weight,
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
        ~weight=node.meta.weight,
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
let resetWeights = tree => AbstractTree.map(withWeight(1.), tree);
