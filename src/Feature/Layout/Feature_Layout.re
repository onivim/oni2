// MODEL

type panel =
  | Left
  | Center(int)
  | Bottom;

type model = {
  tree: Layout.t(int),
  uncommittedTree: [
    | `Resizing(Layout.t(int))
    | `Maximized(Layout.t(int))
    | `None
  ],
};

let initial = id => {tree: Layout.singleton(id), uncommittedTree: `None};

let activeTree = model =>
  switch (model.uncommittedTree) {
  | `Resizing(tree)
  | `Maximized(tree) => tree
  | `None => model.tree
  };

let updateTree = (f, model) => {
  tree: f(activeTree(model)),
  uncommittedTree: `None,
};

let windows = model => Layout.windows(activeTree(model));
let addWindow = (direction, focus) =>
  updateTree(Layout.addWindow(direction, focus));
let insertWindow = (target, direction, focus) =>
  updateTree(Layout.insertWindow(target, direction, focus));
let removeWindow = target => updateTree(Layout.removeWindow(target));

let move = (focus, dirX, dirY, layout) => {
  let positioned = Positioned.fromLayout(0, 0, 200, 200, layout);

  Positioned.move(focus, dirX, dirY, positioned)
  |> Option.value(~default=focus);
};

let moveLeft = current => move(current, -1, 0);
let moveRight = current => move(current, 1, 0);
let moveUp = current => move(current, 0, -1);
let moveDown = current => move(current, 0, 1);

// UPDATE

[@deriving show({with_path: false})]
type command =
  | MoveLeft
  | MoveRight
  | MoveUp
  | MoveDown
  | RotateForward
  | RotateBackward
  | DecreaseSize
  | IncreaseSize
  | DecreaseHorizontalSize
  | IncreaseHorizontalSize
  | DecreaseVerticalSize
  | IncreaseVerticalSize
  | IncreaseWindowSize([ | `Up | `Down | `Left | `Right])
  | DecreaseWindowSize([ | `Up | `Down | `Left | `Right])
  | Maximize
  | MaximizeHorizontal
  | MaximizeVertical
  | ToggleMaximize
  | ResetSizes;

[@deriving show({with_path: false})]
type msg =
  | SplitDragged({
      path: list(int),
      delta: float,
    })
  | DragComplete
  | Command(command);

type outmsg =
  | Nothing
  | Focus(panel);

let rotate = (direction, focus, model) => {
  ...model,
  tree: Layout.rotate(direction, focus, activeTree(model)),
};

let resizeWindowByAxis = (direction, focus, delta, model) => {
  ...model,
  tree:
    Layout.resizeWindowByAxis(direction, focus, delta, activeTree(model)),
};

let resizeWindowByDirection = (direction, focus, delta, model) => {
  ...model,
  tree:
    Layout.resizeWindowByDirection(
      direction,
      focus,
      delta,
      activeTree(model),
    ),
};

let resetWeights = model => {
  tree: Layout.resetWeights(activeTree(model)),
  uncommittedTree: `None,
};

let maximize = (~direction=?, targetId, model) => {
  ...model,
  uncommittedTree:
    `Maximized(Layout.maximize(~direction?, targetId, activeTree(model))),
};

let update = (~focus, model, msg) => {
  switch (msg) {
  | SplitDragged({path, delta}) =>
    let model =
      switch (model.uncommittedTree) {
      | `Maximized(tree) => {...model, tree}
      | `Resizing(_)
      | `None => model
      };
    (
      {
        ...model,
        uncommittedTree:
          `Resizing(Layout.resizeSplit(~path, ~delta, model.tree)),
      },
      Nothing,
    );

  | DragComplete => (updateTree(Fun.id, model), Nothing)

  | Command(MoveLeft) =>
    switch (focus) {
    | Some(Center(focus)) =>
      let newFocus = model |> activeTree |> moveLeft(focus);
      if (newFocus == focus) {
        (model, Focus(Left));
      } else {
        (model, Focus(Center(newFocus)));
      };

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(MoveRight) =>
    switch (focus) {
    | Some(Center(focus)) =>
      let newFocus = model |> activeTree |> moveRight(focus);
      (model, Focus(Center(newFocus)));

    | Some(Left) =>
      let focus = model |> activeTree |> Layout.leftmost;
      (model, Focus(Center(focus)));

    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(MoveUp) =>
    switch (focus) {
    | Some(Center(focus)) =>
      let newFocus = model |> activeTree |> moveUp(focus);
      (model, Focus(Center(newFocus)));

    | Some(Bottom) =>
      let focus = model |> activeTree |> Layout.bottommost;
      (model, Focus(Center(focus)));

    | Some(Left)
    | None => (model, Nothing)
    }

  | Command(MoveDown) =>
    switch (focus) {
    | Some(Center(focus)) =>
      let newFocus = model |> activeTree |> moveDown(focus);
      if (newFocus == focus) {
        (model, Focus(Bottom));
      } else {
        (model, Focus(Center(newFocus)));
      };

    | Some(Left) => (model, Focus(Bottom))

    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(RotateForward) =>
    switch (focus) {
    | Some(Center(focus)) => (rotate(`Forward, focus, model), Nothing)

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(RotateBackward) =>
    switch (focus) {
    | Some(Center(focus)) => (rotate(`Backward, focus, model), Nothing)

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(DecreaseSize) =>
    switch (focus) {
    | Some(Center(focus)) => (
        model
        |> resizeWindowByAxis(`Horizontal, focus, 0.95)
        |> resizeWindowByAxis(`Vertical, focus, 0.95),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(IncreaseSize) =>
    switch (focus) {
    | Some(Center(focus)) => (
        model
        |> resizeWindowByAxis(`Horizontal, focus, 1.05)
        |> resizeWindowByAxis(`Vertical, focus, 1.05),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(DecreaseHorizontalSize) =>
    switch (focus) {
    | Some(Center(focus)) => (
        model |> resizeWindowByAxis(`Horizontal, focus, 0.95),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(IncreaseHorizontalSize) =>
    switch (focus) {
    | Some(Center(focus)) => (
        model |> resizeWindowByAxis(`Horizontal, focus, 1.05),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(DecreaseVerticalSize) =>
    switch (focus) {
    | Some(Center(focus)) => (
        model |> resizeWindowByAxis(`Vertical, focus, 0.95),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(IncreaseVerticalSize) =>
    switch (focus) {
    | Some(Center(focus)) => (
        model |> resizeWindowByAxis(`Vertical, focus, 1.05),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(IncreaseWindowSize(direction)) =>
    switch (focus) {
    | Some(Center(focus)) => (
        model |> resizeWindowByDirection(direction, focus, 1.05),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(DecreaseWindowSize(direction)) =>
    switch (focus) {
    | Some(Center(focus)) => (
        model |> resizeWindowByDirection(direction, focus, 0.95),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(Maximize) =>
    switch (focus) {
    | Some(Center(focus)) => (maximize(focus, model), Nothing)

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(MaximizeHorizontal) =>
    switch (focus) {
    | Some(Center(focus)) => (
        maximize(~direction=`Horizontal, focus, model),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(MaximizeVertical) =>
    switch (focus) {
    | Some(Center(focus)) => (
        maximize(~direction=`Vertical, focus, model),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(ToggleMaximize) =>
    let model =
      switch (model.uncommittedTree) {
      | `Maximized(_) => {...model, uncommittedTree: `None}

      | _ =>
        switch (focus) {
        | Some(Center(focus)) => maximize(focus, model)

        | Some(Left)
        | Some(Bottom)
        | None => model
        }
      };
    (model, Nothing);

  | Command(ResetSizes) => (resetWeights(model), Nothing)
  };
};
// VIEW

module View = {
  module Local = {
    module Layout = Layout;
  };
  open Revery;
  open UI;

  module Constants = {
    let handleSize = 10;
  };

  module Styles = {
    open Style;

    let container = [flexGrow(1), flexDirection(`Row)];

    let verticalHandle = (node: Positioned.t(_)) => [
      cursor(MouseCursors.horizontalResize),
      position(`Absolute),
      left(node.meta.x + node.meta.width - Constants.handleSize / 2),
      top(node.meta.y),
      width(Constants.handleSize),
      height(node.meta.height),
    ];

    let horizontalHandle = (node: Positioned.t(_)) => [
      cursor(MouseCursors.verticalResize),
      position(`Absolute),
      left(node.meta.x),
      top(node.meta.y + node.meta.height - Constants.handleSize / 2),
      width(node.meta.width),
      height(Constants.handleSize),
    ];
  };

  let component = React.Expert.component("handleView");
  let handleView =
      (~direction, ~node: Positioned.t(_), ~onDrag, ~onDragComplete, ()) =>
    component(hooks => {
      let ((captureMouse, _state), hooks) =
        Hooks.mouseCapture(
          ~onMouseMove=
            ((originX, originY), evt) => {
              let delta =
                switch (direction) {
                | `Vertical => evt.mouseX -. originX
                | `Horizontal => evt.mouseY -. originY
                };

              onDrag(delta);
              Some((originX, originY));
            },
          ~onMouseUp=
            (_, _) => {
              onDragComplete();
              None;
            },
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

  let rec nodeView =
          (
            ~theme,
            ~path=[],
            ~node: Positioned.t(_),
            ~renderWindow,
            ~dispatch,
            (),
          ) => {
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
              SplitDragged({
                path: List.rev(path),
                delta: delta /. float(total) // normalized
              }),
            );
          };

          let onDragComplete = () => dispatch(DragComplete);

          [
            <nodeView theme path node renderWindow dispatch />,
            <handleView direction node onDrag onDragComplete />,
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

      let tree = activeTree(model);

      let children =
        switch (maybeDimensions) {
        | Some((width, height)) =>
          let positioned = Positioned.fromLayout(0, 0, width, height, tree);

          <nodeView theme node=positioned renderWindow dispatch />;

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

  module EditorTab = EditorTab;
};

module Commands = {
  open Feature_Commands.Schema;

  let rotateForward =
    define(
      ~category="View",
      ~title="Rotate Windows (Forwards)",
      "view.rotateForward",
      Command(RotateForward),
    );
  let rotateBackward =
    define(
      ~category="View",
      ~title="Rotate Windows (Backwards)",
      "view.rotateBackward",
      Command(RotateBackward),
    );

  let moveLeft =
    define(
      ~category="View",
      ~title="Move Window Focus Left",
      "window.moveLeft",
      Command(MoveLeft),
    );
  let moveRight =
    define(
      ~category="View",
      ~title="Move Window Focus Right",
      "window.moveRight",
      Command(MoveRight),
    );
  let moveUp =
    define(
      ~category="View",
      ~title="Move Window Focus Up",
      "window.moveUp",
      Command(MoveUp),
    );
  let moveDown =
    define(
      ~category="View",
      ~title="Move Window Focus Down",
      "window.moveDown",
      Command(MoveDown),
    );

  let decreaseSize =
    define(
      ~category="View",
      ~title="Decrease Current Window/View Size",
      "workbench.action.decreaseViewSize",
      Command(DecreaseSize),
    );
  let increaseSize =
    define(
      ~category="View",
      ~title="Increase Current Window/View Size",
      "workbench.action.increaseViewSize",
      Command(IncreaseSize),
    );

  let decreaseHorizontalSize =
    define(
      ~category="View",
      ~title="Decrease Horizontal Window Size",
      "vim.decreaseHorizontalWindowSize",
      Command(DecreaseHorizontalSize),
    );
  let increaseHorizontalSize =
    define(
      ~category="View",
      ~title="Increase Horizontal Window Size",
      "vim.increaseHorizontalWindowSize",
      Command(IncreaseHorizontalSize),
    );
  let decreaseVerticalSize =
    define(
      ~category="View",
      ~title="Decrease Vertical Window Size",
      "vim.decreaseVerticalWindowSize",
      Command(DecreaseVerticalSize),
    );
  let increaseVerticalSize =
    define(
      ~category="View",
      ~title="Increase Vertical Window Size",
      "vim.increaseVerticalWindowSize",
      Command(IncreaseVerticalSize),
    );

  let increaseWindowSizeUp =
    define(
      ~category="View",
      ~title="Increase Window Size Up",
      "vim.increaseWindowSizeUp",
      Command(IncreaseWindowSize(`Up)),
    );
  let decreaseWindowSizeUp =
    define(
      ~category="View",
      ~title="Decrease Window Size Up",
      "vim.decreaseWindowSizeUp",
      Command(DecreaseWindowSize(`Up)),
    );
  let increaseWindowSizeDown =
    define(
      ~category="View",
      ~title="Increase Window Size Down",
      "vim.increaseWindowSizeDown",
      Command(IncreaseWindowSize(`Down)),
    );
  let decreaseWindowSizeDown =
    define(
      ~category="View",
      ~title="Decrease Window Size Down",
      "vim.decreaseWindowSizeDown",
      Command(DecreaseWindowSize(`Down)),
    );
  let increaseWindowSizeLeft =
    define(
      ~category="View",
      ~title="Increase Window Size Left",
      "vim.increaseWindowSizeLeft",
      Command(IncreaseWindowSize(`Left)),
    );
  let decreaseWindowSizeLeft =
    define(
      ~category="View",
      ~title="Decrease Window Size Left",
      "vim.decreaseWindowSizeLeft",
      Command(DecreaseWindowSize(`Left)),
    );
  let increaseWindowSizeRight =
    define(
      ~category="View",
      ~title="Increase Window Size Right",
      "vim.increaseWindowSizeRight",
      Command(IncreaseWindowSize(`Right)),
    );
  let decreaseWindowSizeRight =
    define(
      ~category="View",
      ~title="Decrease Window Size Right",
      "vim.decreaseWindowSizeRight",
      Command(DecreaseWindowSize(`Right)),
    );

  let maximize =
    define(
      ~category="View",
      ~title="Maximize Editor Group",
      "workbench.action.maximizeEditor",
      Command(Maximize),
    );
  let maximizeHorizontal =
    define(
      ~category="View",
      ~title="Maximize Editor Group Horizontally",
      "vim.maximizeWindowWidth",
      Command(MaximizeHorizontal),
    );
  let maximizeVertical =
    define(
      ~category="View",
      ~title="Maximize Editor Group Vertically",
      "vim.maximizeWindowHeight",
      Command(MaximizeVertical),
    );
  let toggleMaximize =
    define(
      ~category="View",
      ~title="Toggle Editor Group Sizes",
      "workbench.action.toggleEditorWidths",
      Command(ToggleMaximize),
    );

  let resetSizes =
    define(
      ~category="View",
      ~title="Reset Window Sizes",
      "workbench.action.evenEditorWidths",
      Command(ResetSizes),
    );
};

module Contributions = {
  let commands =
    Commands.[
      rotateForward,
      rotateBackward,
      moveLeft,
      moveRight,
      moveUp,
      moveDown,
      increaseSize,
      decreaseSize,
      increaseHorizontalSize,
      decreaseHorizontalSize,
      increaseVerticalSize,
      decreaseVerticalSize,
      increaseWindowSizeUp,
      decreaseWindowSizeUp,
      increaseWindowSizeDown,
      decreaseWindowSizeDown,
      increaseWindowSizeLeft,
      decreaseWindowSizeLeft,
      increaseWindowSizeRight,
      decreaseWindowSizeRight,
      maximize,
      maximizeHorizontal,
      maximizeVertical,
      toggleMaximize,
      resetSizes,
    ];
};
