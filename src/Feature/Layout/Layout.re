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
         |> List.fold_left((+.), 0.);

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
        * shiftWeightFromLeft
        *
        * Shifts weight to `nodes[index]` from the nodes left of it, consuming
        * the nearest first `delta` is the share of the total weight of the
        * target and siblings combined.
        */
       let shiftWeightFromLeft = (~delta, index, nodes) => {
         let nodes = Array.of_list(nodes);

         let reclaimed = reclaimLeft(~limit=delta, index, nodes);

         let node = nodes[index];
         nodes[index] = node |> withWeight(node.meta.weight +. reclaimed);

         Array.to_list(nodes);
       };

       let%test_module "shiftWeightFromLeft" =
         (module
          {
            let weights = List.map(node => node.meta.weight);
            let weights_int =
              List.map(node => int_of_float(node.meta.weight *. 100.));

            let%test "positive delta" = {
              let initial = [window(1), window(2), window(3)];

              let actual = initial |> shiftWeightFromLeft(~delta=0.05, 1);

              weights(actual) == [0.95, 1.05, 1.];
            };

            let%test "negative delta" = {
              let initial = [window(1), window(2), window(3)];

              let actual = initial |> shiftWeightFromLeft(~delta=-0.05, 1);

              weights(actual) == [1.05, 0.95, 1.];
            };

            let%test "target below minimum" = {
              let initial = [
                window(~weight=0.1, 1),
                window(~weight=0.1, 2),
                window(3),
              ];

              let actual = initial |> shiftWeightFromLeft(~delta=0.05, 1);

              weights(actual) == weights(initial);
            };

            // TODO? minimum not respected with negative delta
            //            let%test "next below minimum - negative delta" = {
            //              let initial = [window(1), window(2), window(~weight=0.1, 3)];
            //
            //              let actual = initial |> shiftWeightFromLeft(~delta=-0.05, 1);
            //
            //              weights(actual) == weights(initial);
            //            };

            let%test "delta too large" = {
              let initial = [window(1), window(2), window(3)];

              let actual = initial |> shiftWeightFromLeft(~delta=4., 1);

              weights_int(actual) == [30, 170, 100];
            };
          });

       /**
        * shiftWeightFromRight
        *
        * Shifts weight to `nodes[index]` from the nodes right of it, consuming
        * the nearest first `delta` is the share of the total weight of the
        * target and siblings combined.
        */
       let shiftWeightFromRight = (~delta, index, nodes) => {
         let nodes = Array.of_list(nodes);

         let reclaimed = reclaimRight(~limit=delta, index, nodes);

         let node = nodes[index];
         nodes[index] = node |> withWeight(node.meta.weight +. reclaimed);

         Array.to_list(nodes);
       };

       let%test_module "shiftWeightFromRight" =
         (module
          {
            let weights = List.map(node => node.meta.weight);
            let weights_int =
              List.map(node => int_of_float(node.meta.weight *. 100.));

            let%test "positive delta" = {
              let initial = [window(1), window(2), window(3)];

              let actual = initial |> shiftWeightFromRight(~delta=0.05, 1);

              weights(actual) == [1., 1.05, 0.95];
            };

            let%test "negative delta" = {
              let initial = [window(1), window(2), window(3)];

              let actual = initial |> shiftWeightFromRight(~delta=-0.05, 1);

              weights(actual) == [1., 0.95, 1.05];
            };

            let%test "target below minimum" = {
              let initial = [
                window(1),
                window(~weight=0.1, 2),
                window(~weight=0.1, 3),
              ];

              let actual = initial |> shiftWeightFromRight(~delta=0.05, 1);

              weights(actual) == weights(initial);
            };

            // TODO? minimum not respected with negative delta
            //            let%test "next below minimum - negative delta" = {
            //              let initial = [window(1), window(2), window(~weight=0.1, 3)];
            //
            //              let actual = initial |> shiftWeightFromRight(~delta=-0.05, 1);
            //
            //              weights(actual) == weights(initial);
            //            };

            let%test "large delta" = {
              let initial = [window(1), window(2), window(3)];

              let actual = initial |> shiftWeightFromRight(~delta=4., 1);

              weights_int(actual) == [100, 170, 30];
            };

            let%test "large weight" = {
              let initial = [window(1), window(2), window(~weight=10., 3)];

              let actual = initial |> shiftWeightFromRight(~delta=0.5, 1);

              weights_int(actual) == [100, 150, 950];
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
    let left = Base.List.take(nodes, i);
    let right = Base.List.drop(nodes, i);
    left @ [node] @ right;
  };

  let insertAfter = (i, node, nodes) => {
    let left = Base.List.take(nodes, i + 1);
    let right = Base.List.drop(nodes, i + 1);
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
     let%test "window - insert vertical - before" = {
       let actual = window(1) |> insertWindow(`Before(1), `Vertical, 2);

       actual == vsplit([window(2), window(1)]);
     };

     let%test "window = insert vertical - after" = {
       let actual = window(1) |> insertWindow(`After(1), `Vertical, 2);

       actual == vsplit([window(1), window(2)]);
     };

     let%test "window - insert horizontal - before" = {
       let actual = window(1) |> insertWindow(`Before(1), `Horizontal, 2);

       actual == hsplit([window(2), window(1)]);
     };

     let%test "window - insert horizontal - after" = {
       let actual = window(1) |> insertWindow(`After(1), `Horizontal, 2);

       actual == hsplit([window(1), window(2)]);
     };

     let%test "vsplit - insert vertical - before" = {
       let actual =
         vsplit([window(1), window(2)])
         |> insertWindow(`Before(2), `Vertical, 3);

       actual == vsplit([window(1), window(3), window(2)]);
     };

     let%test "vsplit = insert vertical - after" = {
       let actual =
         vsplit([window(1), window(2)])
         |> insertWindow(`After(2), `Vertical, 3);

       actual == vsplit([window(1), window(2), window(3)]);
     };

     let%test "vsplit - insert horizontal - before" = {
       let actual =
         vsplit([window(1), window(2)])
         |> insertWindow(`Before(2), `Horizontal, 3);

       actual == vsplit([window(1), hsplit([window(3), window(2)])]);
     };

     let%test "vsplit = insert horizontal - after" = {
       let actual =
         vsplit([window(1), window(2)])
         |> insertWindow(`After(2), `Horizontal, 3);

       actual == vsplit([window(1), hsplit([window(2), window(3)])]);
     };

     let%test "hsplit - insert vertical - before" = {
       let actual =
         hsplit([window(1), window(2)])
         |> insertWindow(`Before(2), `Vertical, 3);

       actual == hsplit([window(1), vsplit([window(3), window(2)])]);
     };

     let%test "hsplit = insert vertical - after" = {
       let actual =
         hsplit([window(1), window(2)])
         |> insertWindow(`After(2), `Vertical, 3);

       actual == hsplit([window(1), vsplit([window(2), window(3)])]);
     };

     let%test "hsplit - insert horizontal - before" = {
       let actual =
         hsplit([window(1), window(2)])
         |> insertWindow(`Before(2), `Horizontal, 3);

       actual == hsplit([window(1), window(3), window(2)]);
     };

     let%test "hsplit = insert horizontal - after" = {
       let actual =
         hsplit([window(1), window(2)])
         |> insertWindow(`After(2), `Horizontal, 3);

       actual == hsplit([window(1), window(2), window(3)]);
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
 * resizeWindowByAxis
 *
 * Resizes the target along the given axis according to the given factor.
 *
 * A factor greater than 1.0 will increase the target size, and less than 1.0
 * will decrease it.
 *
 * Space will be reclaimed rightwards/downwards before leftwards/upwards.
 */
let resizeWindowByAxis = (resizeDirection, targetId, factor, node) => {
  let inflate = (i, nodes) => {
    let total = totalWeight(nodes);
    let delta = total *. factor -. total;

    let nodes = Array.of_list(nodes);
    let reclaimed = reclaimRight(~limit=delta, i, nodes);
    let reclaimed =
      reclaimLeft(~limit=delta -. reclaimed, i, nodes) +. reclaimed;
    let node = nodes[i];
    nodes[i] = node |> withWeight(node.meta.weight +. reclaimed);
    Array.to_list(nodes);
  };

  let rec loop = (path, node) =>
    switch (path, node.kind) {
    | ([], _) => node // shouldn't happen

    | ([index], `Split(direction, children))
        when direction != resizeDirection =>
      node |> withChildren(inflate(index, children))

    | ([parentIndex, _], `Split(direction, children))
        when direction != resizeDirection =>
      node |> withChildren(inflate(parentIndex, children))

    | ([index, ...rest], `Split(_, children)) =>
      let children =
        List.mapi(
          (i, child) => i == index ? loop(rest, child) : child,
          children,
        );
      node |> withChildren(children);

    | _ => raise(Invalid_argument("path"))
    };

  switch (AbstractTree.path(targetId, node)) {
  | Some(path) => loop(path, node)
  | None => node
  };
};

let%test_module "resizeWindowByAxis" =
  (module
   {
     let rec compareNode = (actual, expected) =>
       if (abs_float(actual.meta.weight -. expected.meta.weight) > 0.001) {
         false;
       } else {
         switch (actual.kind, expected.kind) {
         | (`Window(aid), `Window(bid)) => aid == bid
         | (`Split(adir, achildren), `Split(bdir, bchildren)) =>
           adir == bdir && List.for_all2(compareNode, achildren, bchildren)
         | _ => false
         };
       };

     let (==) = compareNode;

     let%test "vsplit - vresize" = {
       let initial = vsplit([window(1), window(2)]);

       let actual = resizeWindowByAxis(`Vertical, 2, 5., initial);

       actual == vsplit([window(1), window(2)]);
     };

     let%test "vsplit - hresize" = {
       let initial = vsplit([window(1), window(2)]);

       let actual = resizeWindowByAxis(`Horizontal, 2, 5., initial);

       actual == vsplit([window(~weight=0.2, 1), window(~weight=1.8, 2)]);
     };

     let%test "hsplit - hresize" = {
       let initial = hsplit([window(1), window(2)]);

       let actual = resizeWindowByAxis(`Horizontal, 2, 5., initial);

       actual == hsplit([window(1), window(2)]);
     };

     let%test "hsplit - vresize" = {
       let initial = hsplit([window(1), window(2)]);

       let actual = resizeWindowByAxis(`Vertical, 2, 5., initial);

       actual == hsplit([window(~weight=0.2, 1), window(~weight=1.8, 2)]);
     };

     let%test "vsplit+hsplit - hresize" = {
       let initial = vsplit([window(1), hsplit([window(2), window(3)])]);

       let actual = resizeWindowByAxis(`Horizontal, 2, 5., initial);

       actual
       == vsplit([
            window(~weight=0.2, 1),
            hsplit(~weight=1.8, [window(2), window(3)]),
          ]);
     };

     let%test "vsplit+hsplit - vresize" = {
       let initial = vsplit([window(1), hsplit([window(2), window(3)])]);

       let actual = resizeWindowByAxis(`Vertical, 2, 5., initial);

       actual
       == vsplit([
            window(1),
            hsplit([window(~weight=1.8, 2), window(~weight=0.2, 3)]),
          ]);
     };

     let%test "hsplit+vsplit - hresize" = {
       let initial = hsplit([window(1), vsplit([window(2), window(3)])]);

       let actual = resizeWindowByAxis(`Horizontal, 2, 5., initial);

       actual
       == hsplit([
            window(1),
            vsplit([window(~weight=1.8, 2), window(~weight=0.2, 3)]),
          ]);
     };

     let%test "hsplit+vsplit - vresize" = {
       let initial = hsplit([window(1), vsplit([window(2), window(3)])]);

       let actual = resizeWindowByAxis(`Vertical, 2, 5., initial);

       actual
       == hsplit([
            window(~weight=0.2, 1),
            vsplit(~weight=1.8, [window(2), window(3)]),
          ]);
     };

     let%test "hsplit+vsplit - vresize - cascading" = {
       let initial =
         hsplit([window(1), vsplit([window(2), window(3)]), window(4)]);

       let actual = resizeWindowByAxis(`Vertical, 2, 5., initial);

       actual
       == hsplit([
            window(~weight=0.23, 1),
            vsplit(~weight=2.47, [window(2), window(3)]),
            window(~weight=0.3, 4),
          ]);
     };
   });

/**
 * resizeWindowByDirection
 *
 * Resizes the target in the fiven direction according to the given factor.
 *
 * A factor greater than 1.0 will increase the target size, and less than 1.0
 * will decrease it.
 */
let resizeWindowByDirection = (resizeDirection, targetId, factor, node) => {
  let inflate = (i, nodes) => {
    let total = totalWeight(nodes);
    let delta = total *. factor -. total;

    let nodes = Array.of_list(nodes);
    let reclaimed =
      switch (resizeDirection) {
      | `Left
      | `Up => reclaimLeft(~limit=delta, i, nodes)
      | `Right
      | `Down => reclaimRight(~limit=delta, i, nodes)
      };
    let node = nodes[i];
    nodes[i] = node |> withWeight(node.meta.weight +. reclaimed);
    Array.to_list(nodes);
  };

  let rec loop = (path, node) =>
    switch (path, node.kind, resizeDirection) {
    | ([index], `Split(`Vertical, children), `Left | `Right)
    | ([index], `Split(`Horizontal, children), `Up | `Down) =>
      node |> withChildren(inflate(index, children))

    | ([parentIndex, _], `Split(`Vertical, children), `Left | `Right)
    | ([parentIndex, _], `Split(`Horizontal, children), `Up | `Down) =>
      node |> withChildren(inflate(parentIndex, children))

    | ([index, ...rest], `Split(_, children), _) =>
      let children =
        List.mapi(
          (i, child) => i == index ? loop(rest, child) : child,
          children,
        );
      node |> withChildren(children);

    | ([], _, _) => node

    | _ => raise(Invalid_argument("path"))
    };

  switch (AbstractTree.path(targetId, node)) {
  | Some(path) => loop(path, node)
  | None => node
  };
};

let%test_module "resizeWindowByDirection" =
  (module
   {
     let rec compareNode = (actual, expected) =>
       if (abs_float(actual.meta.weight -. expected.meta.weight) > 0.001) {
         false;
       } else {
         switch (actual.kind, expected.kind) {
         | (`Window(aid), `Window(bid)) => aid == bid
         | (`Split(adir, achildren), `Split(bdir, bchildren)) =>
           adir == bdir && List.for_all2(compareNode, achildren, bchildren)
         | _ => false
         };
       };

     let (==) = compareNode;

     let%test "vsplit - up" = {
       let initial = vsplit([window(1), window(2), window(3)]);

       let actual = resizeWindowByDirection(`Up, 2, 5., initial);

       actual == vsplit([window(1), window(2), window(3)]);
     };

     let%test "vsplit - down" = {
       let initial = vsplit([window(1), window(2), window(3)]);

       let actual = resizeWindowByDirection(`Down, 2, 5., initial);

       actual == vsplit([window(1), window(2), window(3)]);
     };

     let%test "vsplit - left" = {
       let initial = vsplit([window(1), window(2), window(3)]);

       let actual = resizeWindowByDirection(`Left, 2, 5., initial);

       actual
       == vsplit([
            window(~weight=0.3, 1),
            window(~weight=1.7, 2),
            window(3),
          ]);
     };

     let%test "vsplit - right" = {
       let initial = vsplit([window(1), window(2), window(3)]);

       let actual = resizeWindowByDirection(`Right, 2, 5., initial);

       actual
       == vsplit([
            window(1),
            window(~weight=1.7, 2),
            window(~weight=0.3, 3),
          ]);
     };

     let%test "hsplit - up" = {
       let initial = hsplit([window(1), window(2), window(3)]);

       let actual = resizeWindowByDirection(`Up, 2, 5., initial);

       actual
       == hsplit([
            window(~weight=0.3, 1),
            window(~weight=1.7, 2),
            window(3),
          ]);
     };

     let%test "hsplit - down" = {
       let initial = hsplit([window(1), window(2), window(3)]);

       let actual = resizeWindowByDirection(`Down, 2, 5., initial);

       actual
       == hsplit([
            window(1),
            window(~weight=1.7, 2),
            window(~weight=0.3, 3),
          ]);
     };

     let%test "hsplit - left" = {
       let initial = hsplit([window(1), window(2), window(3)]);

       let actual = resizeWindowByDirection(`Left, 2, 5., initial);

       actual == hsplit([window(1), window(2), window(3)]);
     };

     let%test "hsplit - right" = {
       let initial = hsplit([window(1), window(2), window(3)]);

       let actual = resizeWindowByDirection(`Right, 2, 5., initial);

       actual == hsplit([window(1), window(2), window(3)]);
     };

     let%test "vsplit+hsplit - up" = {
       let initial =
         vsplit([
           window(1),
           hsplit([window(2), window(3), window(4)]),
           window(5),
         ]);

       let actual = resizeWindowByDirection(`Up, 3, 5., initial);

       actual
       == vsplit([
            window(1),
            hsplit([
              window(~weight=0.3, 2),
              window(~weight=1.7, 3),
              window(4),
            ]),
            window(5),
          ]);
     };

     let%test "vsplit+hsplit - down" = {
       let initial =
         vsplit([
           window(1),
           hsplit([window(2), window(3), window(4)]),
           window(5),
         ]);

       let actual = resizeWindowByDirection(`Down, 3, 5., initial);

       actual
       == vsplit([
            window(1),
            hsplit([
              window(2),
              window(~weight=1.7, 3),
              window(~weight=0.3, 4),
            ]),
            window(5),
          ]);
     };

     let%test "vsplit+hsplit - left" = {
       let initial =
         vsplit([
           window(1),
           hsplit([window(2), window(3), window(4)]),
           window(5),
         ]);

       let actual = resizeWindowByDirection(`Left, 3, 5., initial);

       actual
       == vsplit([
            window(~weight=0.3, 1),
            hsplit(~weight=1.7, [window(2), window(3), window(4)]),
            window(5),
          ]);
     };

     let%test "vsplit+hsplit - right" = {
       let initial =
         vsplit([
           window(1),
           hsplit([window(2), window(3), window(4)]),
           window(5),
         ]);

       let actual = resizeWindowByDirection(`Right, 3, 5., initial);

       actual
       == vsplit([
            window(1),
            hsplit(~weight=1.7, [window(2), window(3), window(4)]),
            window(~weight=0.3, 5),
          ]);
     };

     let%test "hsplit+vsplit - up" = {
       let initial =
         hsplit([
           window(1),
           vsplit([window(2), window(3), window(4)]),
           window(5),
         ]);

       let actual = resizeWindowByDirection(`Up, 3, 5., initial);

       actual
       == hsplit([
            window(~weight=0.3, 1),
            vsplit(~weight=1.7, [window(2), window(3), window(4)]),
            window(5),
          ]);
     };

     let%test "hsplit+vsplit - down" = {
       let initial =
         hsplit([
           window(1),
           vsplit([window(2), window(3), window(4)]),
           window(5),
         ]);

       let actual = resizeWindowByDirection(`Down, 3, 5., initial);

       actual
       == hsplit([
            window(1),
            vsplit(~weight=1.7, [window(2), window(3), window(4)]),
            window(~weight=0.3, 5),
          ]);
     };

     let%test "hsplit+vsplit - left" = {
       let initial =
         hsplit([
           window(1),
           vsplit([window(2), window(3), window(4)]),
           window(5),
         ]);

       let actual = resizeWindowByDirection(`Left, 3, 5., initial);

       actual
       == hsplit([
            window(1),
            vsplit([
              window(~weight=0.3, 2),
              window(~weight=1.7, 3),
              window(4),
            ]),
            window(5),
          ]);
     };

     let%test "hsplit+vsplit - right" = {
       let initial =
         hsplit([
           window(1),
           vsplit([window(2), window(3), window(4)]),
           window(5),
         ]);

       let actual = resizeWindowByDirection(`Right, 3, 5., initial);

       actual
       == hsplit([
            window(1),
            vsplit([
              window(2),
              window(~weight=1.7, 3),
              window(~weight=0.3, 4),
            ]),
            window(5),
          ]);
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
    | `Split(_, children) =>
      let children =
        if (delta > 0.) {
          shiftWeightFromRight(
            ~delta=totalWeight(children) *. delta,
            index,
            children,
          );
        } else {
          shiftWeightFromLeft(
            ~delta=totalWeight(children) *. (-. delta),
            index + 1,
            children,
          );
        };
      node |> withChildren(children);

    | `Window(_) => node
    }

  | [index, ...rest] =>
    switch (node.kind) {
    | `Split(_, children) =>
      let children =
        List.mapi(
          (i, child) =>
            i == index ? resizeSplit(~path=rest, ~delta, child) : child,
          children,
        );
      node |> withChildren(children);

    | `Window(_) => node
    }
  };
};

/**
 * resetWeights
 */
let resetWeights = tree => AbstractTree.map(withWeight(1.), tree);

/**
 * maximize
 */
let maximize = (~direction as targetDirection=?, targetId, tree) => {
  let rec loop = (path, node) =>
    switch (path, node.kind) {
    | ([index, ...rest], `Split(direction, children)) =>
      let children =
        List.mapi(
          (i, child) =>
            if (i == index && targetDirection != Some(direction)) {
              child |> withWeight(10.) |> loop(rest);
            } else {
              child |> withWeight(1.) |> loop(rest);
            },
          children,
        );
      node |> withChildren(children);

    | _ => node
    };

  switch (AbstractTree.path(targetId, tree)) {
  | Some(path) => tree |> loop(path)
  | None => tree
  };
};

let%test_module "maximize" =
  (module
   {
     let rec compareNode = (actual, expected) =>
       if (abs_float(actual.meta.weight -. expected.meta.weight) > 0.001) {
         false;
       } else {
         switch (actual.kind, expected.kind) {
         | (`Window(aid), `Window(bid)) => aid == bid
         | (`Split(adir, achildren), `Split(bdir, bchildren)) =>
           adir == bdir && List.for_all2(compareNode, achildren, bchildren)
         | _ => false
         };
       };

     let (==) = compareNode;

     let%test "vsplit - both" = {
       let initial = vsplit([window(1), window(2)]);

       let actual = maximize(2, initial);

       actual == vsplit([window(1), window(~weight=10., 2)]);
     };

     let%test "vsplit - horizontal" = {
       let initial = vsplit([window(1), window(2)]);

       let actual = maximize(~direction=`Horizontal, 2, initial);

       actual == vsplit([window(1), window(~weight=10., 2)]);
     };

     let%test "vsplit - vertical" = {
       let initial = vsplit([window(1), window(2)]);

       let actual = maximize(~direction=`Vertical, 2, initial);

       actual == vsplit([window(1), window(2)]);
     };

     let%test "hsplit - both" = {
       let initial = hsplit([window(1), window(2)]);

       let actual = maximize(2, initial);

       actual == hsplit([window(1), window(~weight=10., 2)]);
     };

     let%test "hsplit - horizontal" = {
       let initial = hsplit([window(1), window(2)]);

       let actual = maximize(~direction=`Horizontal, 2, initial);

       actual == hsplit([window(1), window(2)]);
     };

     let%test "hsplit - vertical" = {
       let initial = hsplit([window(1), window(2)]);

       let actual = maximize(~direction=`Vertical, 2, initial);

       actual == hsplit([window(1), window(~weight=10., 2)]);
     };

     let%test "vsplit+hsplit - both" = {
       let initial = vsplit([window(1), hsplit([window(2), window(3)])]);

       let actual = maximize(2, initial);

       actual
       == vsplit([
            window(1),
            hsplit(~weight=10., [window(~weight=10., 2), window(3)]),
          ]);
     };

     let%test "vsplit+hsplit - horizontal" = {
       let initial = vsplit([window(1), hsplit([window(2), window(3)])]);

       let actual = maximize(~direction=`Horizontal, 2, initial);

       actual
       == vsplit([
            window(1),
            hsplit(~weight=10., [window(2), window(3)]),
          ]);
     };

     let%test "vsplit+hsplit - vertical" = {
       let initial = vsplit([window(1), hsplit([window(2), window(3)])]);

       let actual = maximize(~direction=`Vertical, 2, initial);

       actual
       == vsplit([window(1), hsplit([window(~weight=10., 2), window(3)])]);
     };

     let%test "hsplit+vsplit - both" = {
       let initial = hsplit([window(1), vsplit([window(2), window(3)])]);

       let actual = maximize(2, initial);

       actual
       == hsplit([
            window(1),
            vsplit(~weight=10., [window(~weight=10., 2), window(3)]),
          ]);
     };

     let%test "hsplit+vsplit - horizontal" = {
       let initial = hsplit([window(1), vsplit([window(2), window(3)])]);

       let actual = maximize(~direction=`Horizontal, 2, initial);

       actual
       == hsplit([window(1), vsplit([window(~weight=10., 2), window(3)])]);
     };

     let%test "hsplit+vsplit - vertical" = {
       let initial = hsplit([window(1), vsplit([window(2), window(3)])]);

       let actual = maximize(~direction=`Vertical, 2, initial);

       actual
       == hsplit([
            window(1),
            vsplit(~weight=10., [window(2), window(3)]),
          ]);
     };
   });

let rec topmost = node =>
  switch (node.kind) {
  | `Window(id) => id
  | `Split(_, [first, ..._]) => topmost(first)
  | `Split(_, []) => failwith("encountered empty split")
  };

let rec bottommost = node =>
  switch (node.kind) {
  | `Window(id) => id
  | `Split(`Horizontal, children) =>
    children |> Base.List.last_exn |> bottommost
  | `Split(`Vertical, [first, ..._]) => bottommost(first)
  | `Split(_, []) => failwith("encountered empty split")
  };

let rec leftmost = node =>
  switch (node.kind) {
  | `Window(id) => id
  | `Split(_, [first, ..._]) => leftmost(first)
  | `Split(_, []) => failwith("encountered empty split")
  };

let rec rightmost = node =>
  switch (node.kind) {
  | `Window(id) => id
  | `Split(`Vertical, children) => children |> Base.List.last_exn |> rightmost
  | `Split(`Horizontal, [first, ..._]) => rightmost(first)
  | `Split(_, []) => failwith("encountered empty split")
  };
