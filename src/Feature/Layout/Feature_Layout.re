// MODEL

type model = Layout.t(int);

let initial = id => Layout.singleton(id);

let windows = Layout.windows;
let addWindow = Layout.addWindow;
let removeWindow = Layout.removeWindow;

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
  | ResetSizes;

[@deriving show({with_path: false})]
type msg =
  | HandleDragged({
      path: list(int),
      delta: float,
    })
  | Command(command);

type outmsg =
  | Nothing
  | Focus(int);

let update = (~focus, model, msg) => {
  switch (msg) {
  | Command(MoveLeft) =>
    switch (focus) {
    | Some(focus) => (model, Focus(moveLeft(focus, model)))
    | None => (model, Nothing)
    }

  | Command(MoveRight) =>
    switch (focus) {
    | Some(focus) => (model, Focus(moveRight(focus, model)))
    | None => (model, Nothing)
    }

  | Command(MoveUp) =>
    switch (focus) {
    | Some(focus) => (model, Focus(moveUp(focus, model)))
    | None => (model, Nothing)
    }

  | Command(MoveDown) =>
    switch (focus) {
    | Some(focus) => (model, Focus(moveDown(focus, model)))
    | None => (model, Nothing)
    }

  | Command(RotateForward) =>
    switch (focus) {
    | Some(focus) => (Layout.rotateForward(focus, model), Nothing)
    | None => (model, Nothing)
    }

  | Command(RotateBackward) =>
    switch (focus) {
    | Some(focus) => (Layout.rotateBackward(focus, model), Nothing)
    | None => (model, Nothing)
    }

  | Command(DecreaseSize) =>
    switch (focus) {
    | Some(focus) => (
        model
        |> Layout.resizeWindow(`Horizontal, focus, 0.95)
        |> Layout.resizeWindow(`Vertical, focus, 0.95),
        Nothing,
      )
    | None => (model, Nothing)
    }

  | Command(IncreaseSize) =>
    switch (focus) {
    | Some(focus) => (
        model
        |> Layout.resizeWindow(`Horizontal, focus, 1.05)
        |> Layout.resizeWindow(`Vertical, focus, 1.05),
        Nothing,
      )
    | None => (model, Nothing)
    }

  | Command(DecreaseHorizontalSize) =>
    switch (focus) {
    | Some(focus) => (
        model |> Layout.resizeWindow(`Horizontal, focus, 0.95),
        Nothing,
      )
    | None => (model, Nothing)
    }

  | Command(IncreaseHorizontalSize) =>
    switch (focus) {
    | Some(focus) => (
        model |> Layout.resizeWindow(`Horizontal, focus, 1.05),
        Nothing,
      )
    | None => (model, Nothing)
    }

  | Command(DecreaseVerticalSize) =>
    switch (focus) {
    | Some(focus) => (
        model |> Layout.resizeWindow(`Vertical, focus, 0.95),
        Nothing,
      )
    | None => (model, Nothing)
    }

  | Command(IncreaseVerticalSize) =>
    switch (focus) {
    | Some(focus) => (
        model |> Layout.resizeWindow(`Vertical, focus, 1.05),
        Nothing,
      )
    | None => (model, Nothing)
    }

  | Command(ResetSizes) => (Layout.resetWeights(model), Nothing)

  | HandleDragged({path, delta}) => (
      Layout.resizeSplit(~path, ~delta, model),
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
  let handleView = (~direction, ~node: Positioned.t(_), ~onDrag, ()) =>
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
          let positioned = Positioned.fromLayout(0, 0, width, height, model);

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
      resetSizes,
    ];
};
