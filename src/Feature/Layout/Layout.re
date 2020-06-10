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

       let reclaim = (~limit, ~start, ~next, ~stopWhen, nodes) => {
         let length = Array.length(nodes);
         let total =
           Array.fold_left(
             (acc, node) => acc +. node.meta.weight,
             0.,
             nodes,
           );
         let minimum = min(0.1 *. total, total /. float(length));

         let rec loop = (i, reclaimed) =>
           if (stopWhen(i)) {
             reclaimed;
           } else {
             let node = nodes[i];
             let remaining = limit -. reclaimed;
             let available = max(0., node.meta.weight -. minimum);
             let reclaim = remaining < available ? remaining : available;

             nodes[i] = node |> withWeight(node.meta.weight -. reclaim);

             loop(next(i), reclaimed +. reclaim);
           };

         loop(start, 0.);
       };

       let reclaimLeft = (~limit, index, nodes) =>
         reclaim(
           ~limit,
           ~start=index - 1,
           ~next=pred,
           ~stopWhen=i => i < 0,
           nodes,
         );

       let%test_module "reclaimLeft" =
         (module
          {
            // Because floats are tricky to compare, this inflates them by a
            // magnitude of two, converts them to int and then compares,
            // effectively comparing each float value with a tolerance of 0.01..
            let (==) = (actual, expected) => {
              let intify = n => int_of_float(n *. 100.);

              let actual = (
                intify(fst(actual)),
                Array.map(node => intify(node.meta.weight), snd(actual)),
              );
              let expected = (
                intify(fst(expected)),
                Array.map(intify, snd(expected)),
              );

              actual == expected;
            };

            let%test "available < limit, target == 1" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimLeft(~limit=10., 1, nodes);

              (reclaimed, nodes) == (0.7, [|0.3, 1., 1.|]);
            };

            let%test "available > limit, target == 1" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimLeft(~limit=0.5, 1, nodes);

              (reclaimed, nodes) == (0.5, [|0.5, 1., 1.|]);
            };

            let%test "available < limit, target == 2" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimLeft(~limit=10., 2, nodes);

              (reclaimed, nodes) == (1.4, [|0.3, 0.3, 1.|]);
            };

            let%test "available > limit, target == 2" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimLeft(~limit=0.5, 2, nodes);

              (reclaimed, nodes) == (0.5, [|1., 0.5, 1.|]);
            };

            let%test "available > limit && single node, target == 2" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimLeft(~limit=1., 2, nodes);

              (reclaimed, nodes) == (1., [|0.7, 0.3, 1.|]);
            };

            let%test "available < limit, target == 0" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimLeft(~limit=10., 0, nodes);

              (reclaimed, nodes) == (0., [|1., 1., 1.|]);
            };

            let%test "available > limit, target == 0" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimLeft(~limit=0.5, 0, nodes);

              (reclaimed, nodes) == (0., [|1., 1., 1.|]);
            };

            let%test "target == -3 (out of bounds)" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimLeft(~limit=10., -3, nodes);

              (reclaimed, nodes) == (0., [|1., 1., 1.|]);
            };

            let%test "target == 6 (out of bounds)" = {
              let nodes = [|window(1), window(2), window(3)|];

              switch (reclaimLeft(~limit=10., 6, nodes)) {
              | _ => false
              | exception (Invalid_argument(_)) => true
              };
            };
          });

       let reclaimRight = (~limit, index, nodes) =>
         reclaim(
           ~limit,
           ~start=index + 1,
           ~next=succ,
           ~stopWhen=i => i >= Array.length(nodes),
           nodes,
         );

       let%test_module "reclaimRight" =
         (module
          {
            // Because floats are tricky to compare, this inflates them by a
            // magnitude of two, converts them to int and then compares,
            // effectively comparing each float value with a tolerance of 0.01..
            let (==) = (actual, expected) => {
              let intify = n => int_of_float(n *. 100.);

              let actual = (
                intify(fst(actual)),
                Array.map(node => intify(node.meta.weight), snd(actual)),
              );
              let expected = (
                intify(fst(expected)),
                Array.map(intify, snd(expected)),
              );

              actual == expected;
            };

            let%test "available < limit, target == 1" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimRight(~limit=10., 1, nodes);

              (reclaimed, nodes) == (0.7, [|1., 1., 0.3|]);
            };

            let%test "available > limit, target == 1" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimRight(~limit=0.5, 1, nodes);

              (reclaimed, nodes) == (0.5, [|1., 1., 0.5|]);
            };

            let%test "available < limit, target == 2" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimRight(~limit=10., 2, nodes);

              (reclaimed, nodes) == (0., [|1., 1., 1.|]);
            };

            let%test "available > limit, target == 2" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimRight(~limit=0.5, 2, nodes);

              (reclaimed, nodes) == (0., [|1., 1., 1.|]);
            };

            let%test "available < limit, target == 0" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimRight(~limit=10., 0, nodes);

              (reclaimed, nodes) == (1.4, [|1., 0.3, 0.3|]);
            };

            let%test "available > limit, target == 0" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimRight(~limit=0.5, 0, nodes);

              (reclaimed, nodes) == (0.5, [|1., 0.5, 1.|]);
            };

            let%test "available > limit && single node, target == 0" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimRight(~limit=1., 0, nodes);

              (reclaimed, nodes) == (1., [|1., 0.3, 0.7|]);
            };

            let%test "target == -3 (out of bounds)" = {
              let nodes = [|window(1), window(2), window(3)|];

              switch (reclaimRight(~limit=10., -3, nodes)) {
              | _ => false
              | exception (Invalid_argument(_)) => true
              };
            };

            let%test "target == 6 (out of bounds)" = {
              let nodes = [|window(1), window(2), window(3)|];

              let reclaimed = reclaimRight(~limit=10., 6, nodes);

              (reclaimed, nodes) == (0., [|1., 1., 1.|]);
            };
          });

       /**
        * shiftWeightLeft
        *
        * Shifts weight from `nodes[index]` to `nodes[index-1]`. `delta` is the
        * share of the total weight of the target and siblings combined.
        */
       let shiftWeightLeft = (~delta, index, nodes) => {
         let nodes = Array.of_list(nodes);

         let reclaimed = reclaimLeft(~limit=delta, index, nodes);

         let node = nodes[index];
         nodes[index] = node |> withWeight(node.meta.weight +. reclaimed);

         Array.to_list(nodes);
       };

       let%test_module "shiftWeightLeft" =
         (module
          {
            let weights = List.map(node => node.meta.weight);
            let weights_int =
              List.map(node => int_of_float(node.meta.weight *. 100.));

            let%test "positive delta" = {
              let initial = [window(1), window(2), window(3)];

              let actual = initial |> shiftWeightLeft(~delta=0.05, 1);

              weights(actual) == [0.95, 1.05, 1.];
            };

            let%test "negative delta" = {
              let initial = [window(1), window(2), window(3)];

              let actual = initial |> shiftWeightLeft(~delta=-0.05, 1);

              weights(actual) == [1.05, 0.95, 1.];
            };

            let%test "target below minimum" = {
              let initial = [
                window(~weight=0.1, 1),
                window(~weight=0.1, 2),
                window(3),
              ];

              let actual = initial |> shiftWeightLeft(~delta=0.05, 1);

              weights(actual) == weights(initial);
            };

            // TODO? minimum not respected with negative delta
            //            let%test "next below minimum - negative delta" = {
            //              let initial = [window(1), window(2), window(~weight=0.1, 3)];
            //
            //              let actual = initial |> shiftWeightLeft(~delta=-0.05, 1);
            //
            //              weights(actual) == weights(initial);
            //            };

            let%test "delta too large" = {
              let initial = [window(1), window(2), window(3)];

              let actual = initial |> shiftWeightLeft(~delta=4., 1);

              weights_int(actual) == [30, 170, 100];
            };
          });

       /**
        * shiftWeightRight
        *
        * Shifts weight from `nodes[index]` to `nodes[index+1]`. `delta` is the
        * share of the total weight of the target and siblings combined.
        */
       let shiftWeightRight = (~delta, index, nodes) => {
         let nodes = Array.of_list(nodes);

         let reclaimed = reclaimRight(~limit=delta, index, nodes);

         let node = nodes[index];
         nodes[index] = node |> withWeight(node.meta.weight +. reclaimed);

         Array.to_list(nodes);
       };

       let%test_module "shiftWeightRight" =
         (module
          {
            let weights = List.map(node => node.meta.weight);
            let weights_int =
              List.map(node => int_of_float(node.meta.weight *. 100.));

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
              let initial = [
                window(1),
                window(~weight=0.1, 2),
                window(~weight=0.1, 3),
              ];

              let actual = initial |> shiftWeightRight(~delta=0.05, 1);

              weights(actual) == weights(initial);
            };

            // TODO? minimum not respected with negative delta
            //            let%test "next below minimum - negative delta" = {
            //              let initial = [window(1), window(2), window(~weight=0.1, 3)];
            //
            //              let actual = initial |> shiftWeightRight(~delta=-0.05, 1);
            //
            //              weights(actual) == weights(initial);
            //            };

            let%test "delta too large" = {
              let initial = [window(1), window(2), window(3)];

              let actual = initial |> shiftWeightRight(~delta=4., 1);

              weights_int(actual) == [100, 170, 30];
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
      let children =
        if (delta > 0.) {
          shiftWeightRight(
            ~delta=totalWeight(children) *. delta,
            index,
            children,
          );
        } else {
          shiftWeightLeft(
            ~delta=totalWeight(children) *. (-. delta),
            index + 1,
            children,
          );
        };

      split(~weight=node.meta.weight, direction, children);

    | `Window(_) => node
    }

  | [index, ...rest] =>
    switch (node.kind) {
    | `Split(direction, children) =>
      split(
        ~weight=node.meta.weight,
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
let resetWeights = tree => AbstractTree.map(withWeight(1.), tree);
