open Utility;

// MODEL

type node('id, 'meta) = {
  meta: 'meta,
  kind: [
    | `Split([ | `Horizontal | `Vertical], list(node('id, 'meta)))
    | `Window('id)
  ],
};

type metadata = {size: float};

type t('id) = node('id, metadata);

type sizedMetadata = {
  x: int,
  y: int,
  width: int,
  height: int,
};

type sized('id) = node('id, sizedMetadata);

module Internal = {
  let contains = (x, y, {meta, _}) => {
    x >= meta.x
    && x <= meta.x
    + meta.width
    && y >= meta.y
    && y <= meta.y
    + meta.height;
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
        (prev, {meta, _}) => {
          let (minX, minY, maxX, maxY, deltaX, deltaY) = prev;

          let newMinX = meta.x < minX ? meta.x : minX;
          let newMinY = meta.y < minY ? meta.y : minY;
          let newMaxX =
            meta.x + meta.width > maxX ? meta.x + meta.width : maxX;
          let newMaxY =
            meta.y + meta.height > maxY ? meta.y + meta.height : maxY;
          let newDeltaX = meta.width / 2 < deltaX ? meta.width / 2 : deltaX;
          let newDeltaY = meta.height / 2 < deltaY ? meta.height / 2 : deltaY;

          (newMinX, newMinY, newMaxX, newMaxY, newDeltaX, newDeltaY);
        },
        (0, 0, 1, 1, 100, 100),
        splits,
      );

    switch (List.find_opt(split => split.kind == `Window(targetId), splits)) {
    | None => None
    | Some({meta, _}) =>
      let curX = ref(meta.x + meta.width / 2);
      let curY = ref(meta.y + meta.height / 2);
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

  let rec rotate = (targetId, f, node) => {
    switch (node.kind) {
    | `Split(direction, children) =>
      let children =
        List.exists(({kind, _}) => kind == `Window(targetId), children)
          ? f(children) : List.map(rotate(targetId, f), children);

      {...node, kind: `Split((direction, children))};

    | `Window(_) => node
    };
  };
};

let empty = {
  meta: {
    size: 1.,
  },
  kind: `Split((`Vertical, [])),
};
let initial = empty;

let windows = tree => {
  let rec traverse = (node, acc) => {
    switch (node.kind) {
    | `Split(_, children) =>
      List.fold_left((acc, child) => traverse(child, acc), acc, children)
    | `Window(id) => [id, ...acc]
    };
  };

  traverse(tree, []);
};

let rec layout = (x, y, width, height, node) => {
  switch (node.kind) {
  | `Split(direction, children) =>
    let totalWeight =
      children
      |> List.map(child => child.meta.size)
      |> List.fold_left((+.), 0.)
      |> max(1.);

    let sizedChildren =
      (
        switch (direction) {
        | `Horizontal =>
          let unitHeight = float(height) /. totalWeight;
          List.fold_left(
            ((y, acc), child) => {
              let height = int_of_float(unitHeight *. child.meta.size);
              let sized = layout(x, y, width, height, child);
              (y + height, [sized, ...acc]);
            },
            (y, []),
            children,
          );

        | `Vertical =>
          let unitWidth = float(width) /. totalWeight;
          List.fold_left(
            ((x, acc), child) => {
              let width = int_of_float(unitWidth *. child.meta.size);
              let sized = layout(x, y, width, height, child);
              (x + width, [sized, ...acc]);
            },
            (x, []),
            children,
          );
        }
      )
      |> snd
      |> List.rev;

    {
      meta: {
        x,
        y,
        width,
        height,
      },
      kind: `Split((direction, sizedChildren)),
    };

  | `Window(id) => {
      meta: {
        x,
        y,
        width,
        height,
      },
      kind: `Window(id),
    }
  };
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

// UPDATE

[@deriving show({with_path: false})]
type msg =
  | HandleDragged({
      path: list(int),
      delta: float,
    })
  | MoveLeft
  | MoveRight
  | MoveUp
  | MoveDown
  | RotateForward
  | RotateBackward;

type outmsg('id) =
  | Nothing
  | Focus('id);

let update = (~focus, model, msg) => {
  switch (msg) {
  | MoveLeft =>
    switch (focus) {
    | Some(focus) => (model, Focus(moveLeft(focus, model)))
    | None => (model, Nothing)
    }

  | MoveRight =>
    switch (focus) {
    | Some(focus) => (model, Focus(moveRight(focus, model)))
    | None => (model, Nothing)
    }

  | MoveUp =>
    switch (focus) {
    | Some(focus) => (model, Focus(moveUp(focus, model)))
    | None => (model, Nothing)
    }

  | MoveDown =>
    switch (focus) {
    | Some(focus) => (model, Focus(moveDown(focus, model)))
    | None => (model, Nothing)
    }

  | RotateForward =>
    switch (focus) {
    | Some(focus) => (rotateForward(focus, model), Nothing)
    | None => (model, Nothing)
    }

  | RotateBackward =>
    switch (focus) {
    | Some(focus) => (rotateBackward(focus, model), Nothing)
    | None => (model, Nothing)
    }

  | HandleDragged({path, delta}) => (
      resizeSplit(~path, ~delta, model),
      Nothing,
    )
  };
};

// VIEW

module View = {
  open Revery;
  open UI;

  module Constants = {
    let handleSize = 10;
  };

  module Styles = {
    open Style;

    let container = [flexGrow(1), flexDirection(`Row)];

    let verticalHandle = (node: sized(_)) => [
      cursor(MouseCursors.horizontalResize),
      position(`Absolute),
      left(node.meta.x + node.meta.width - Constants.handleSize / 2),
      top(node.meta.y),
      width(Constants.handleSize),
      height(node.meta.height),
    ];

    let horizontalHandle = (node: sized(_)) => [
      cursor(MouseCursors.verticalResize),
      position(`Absolute),
      left(node.meta.x),
      top(node.meta.y + node.meta.height - Constants.handleSize / 2),
      width(node.meta.width),
      height(Constants.handleSize),
    ];
  };

  let component = React.Expert.component("handleView");
  let handleView = (~direction, ~node: sized(_), ~onDrag, ()) =>
    component(hooks => {
      let ((captureMouse, _state), hooks) =
        Hooks.mouseCapture(
          ~onMouseMove=
            ((lastX, lastY), evt) => {
              let delta =
                switch (direction) {
                | `Vertical => evt.mouseX -. lastX
                | `Horizontal => evt.mouseY -. lastY
                };

              onDrag(delta);
              Some((evt.mouseX, evt.mouseY));
            },
          ~onMouseUp=(_, _) => None,
          (),
          hooks,
        );

      let onMouseDown = (evt: NodeEvents.mouseButtonEventParams) => {
        captureMouse((evt.mouseX, evt.mouseY));
      };

      (
        <View
          onMouseDown
          style={
            direction == `Vertical
              ? Styles.verticalHandle(node) : Styles.horizontalHandle(node)
          }
        />,
        hooks,
      );
    });

  let rec nodeView = (~theme, ~path=[], ~node, ~renderWindow, ~dispatch, ()) => {
    switch (node.kind) {
    | `Split(direction, children) =>
      let parent = node;

      let rec loop = (index, children) => {
        let path = [index, ...path];

        switch (children) {
        | [] => []
        | [node] => [<nodeView theme path node renderWindow dispatch />]

        | [node, ...[_, ..._] as rest] =>
          let onDrag = delta => {
            let total =
              direction == `Vertical ? parent.meta.width : parent.meta.height;
            dispatch(
              HandleDragged({
                path: List.rev(path),
                delta: delta /. float(total) // normalized
              }),
            );
          };
          [
            <nodeView theme path node renderWindow dispatch />,
            <handleView direction node onDrag />,
            ...loop(index + 1, rest),
          ];
        };
      };

      loop(0, children) |> React.listToElement;

    | `Window(id) =>
      <View
        style=Style.[
          position(`Absolute),
          left(node.meta.x),
          top(node.meta.y),
          width(node.meta.width),
          height(node.meta.height),
        ]>
        {renderWindow(id)}
      </View>
    };
  };

  let component = React.Expert.component("Feature_Layout.View");
  let make = (~children as renderWindow, ~model, ~theme, ~dispatch, ()) =>
    component(hooks => {
      let ((maybeDimensions, setDimensions), hooks) =
        Hooks.state(None, hooks);
      let children =
        switch (maybeDimensions) {
        | Some((width, height)) =>
          let sizedLayout = layout(0, 0, width, height, model);

          <nodeView theme node=sizedLayout renderWindow dispatch />;

        | None => React.empty
        };

      (
        <View
          onDimensionsChanged={dim =>
            setDimensions(_ => Some((dim.width, dim.height)))
          }
          style=Styles.container>
          children
        </View>,
        hooks,
      );
    });
};
