include AbstractTree;

type metadata = {size: float};

type t('id) = AbstractTree.node('id, metadata);

let empty = {
  meta: {
    size: 1.,
  },
  kind: `Split((`Vertical, [])),
};

let singleton = id => {
  meta: {
    size: 1.,
  },
  kind: `Split((`Vertical, [{
                              meta: {
                                size: 1.,
                              },
                              kind: `Window(id),
                            }])),
};

let addWindow = (~target=None, ~position, direction, id, tree) => {
  let newWindow = {
    meta: {
      size: 1.,
    },
    kind: `Window(id),
  };
  switch (target) {
  | Some(targetId) =>
    let rec traverse = node => {
      switch (node.kind) {
      | `Split(_, []) => {...node, kind: `Window(id)} // HACK: to work around this being intially called with an idea that doesn't yet exist in the tree
      | `Split(thisDirection, children) when thisDirection == direction =>
        let onMatch = child =>
          switch (position) {
          | `Before => [newWindow, child]
          | `After => [child, newWindow]
          };
        {
          ...node,
          kind:
            `Split((
              thisDirection,
              traverseChildren(~onMatch, [], children),
            )),
        };

      | `Split(thisDirection, children) =>
        let onMatch = ({meta, kind}) => {
          let children =
            switch (position) {
            | `Before => [newWindow, {
                                       meta: {
                                         size: 1.,
                                       },
                                       kind,
                                     }]
            | `After => [{
                           meta: {
                             size: 1.,
                           },
                           kind,
                         }, newWindow]
            };

          [{meta, kind: `Split((direction, children))}];
        };

        {
          ...node,
          kind:
            `Split((
              thisDirection,
              traverseChildren(~onMatch, [], children),
            )),
        };

      | `Window(id) when id == targetId =>
        let children =
          switch (position) {
          | `Before => [newWindow, {
                                     meta: {
                                       size: 1.,
                                     },
                                     kind: node.kind,
                                   }]
          | `After => [{
                         meta: {
                           size: 1.,
                         },
                         kind: node.kind,
                       }, newWindow]
          };

        {meta: node.meta, kind: `Split((direction, children))};

      | `Window(_) => node
      };
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
    | `Split(_, []) => {...tree, kind: `Window(id)}
    | `Split(d, children) => {
        ...tree,
        kind: `Split((d, [newWindow, ...children])),
      }
    | `Window(id) => {
        ...tree,
        kind:
          `Split((
            direction,
            [newWindow, {
                          meta: {
                            size: 1.,
                          },
                          kind: `Window(id),
                        }],
          )),
      }
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
        Some({...node, kind: `Split((direction, newChildren))})
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
          {
            meta: {
              size: node.meta.size *. factor,
            },
            kind: `Split((dir, children)),
          },
        )

      | _ => (result, {...node, kind: `Split((dir, children))})
      };

    | `Window(id) when id == targetId =>
      if (parentDirection == Some(direction)) {
        (`NotAdjusted, node);
      } else {
        (`Adjusted, {
                      ...node,
                      meta: {
                        size: node.meta.size *. factor,
                      },
                    });
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
              {
                ...node,
                meta: {
                  size: weight +. deltaWeight,
                },
              },
              {
                ...next,
                meta: {
                  size: nextWeight -. deltaWeight,
                },
              },
              ...rest,
            ];
          }
        | [node, ...rest] => [node, ...resizeChildren(i + 1, rest)]
      );

      {...node, kind: `Split((direction, resizeChildren(0, children)))};

    | `Window(_) => node
    }
  | [index, ...rest] =>
    switch (node.kind) {
    | `Split(direction, children) => {
        ...node,
        kind:
          `Split((
            direction,
            List.mapi(
              (i, child) =>
                i == index ? resizeSplit(~path=rest, ~delta, child) : child,
              children,
            ),
          )),
      }
    | `Window(_) => node
    }
  };
};

let rec resetWeights = node =>
  switch (node.kind) {
  | `Split(direction, children) => {
      meta: {
        size: 1.,
      },
      kind: `Split((direction, List.map(resetWeights, children))),
    }
  | `Window(_) => {
      ...node,
      meta: {
        size: 1.,
      },
    }
  };
