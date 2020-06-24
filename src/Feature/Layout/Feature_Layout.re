// MODEL

include Model;

// UPDATE

open Msg;

[@deriving show({with_path: false})]
type msg = Msg.t;

type outmsg =
  | Nothing
  | SplitAdded
  | RemoveLastBlocked
  | Focus(panel);

open {
       let rotate = direction =>
         updateActiveLayout(layout =>
           {
             ...layout,
             tree:
               Layout.rotate(
                 direction,
                 layout.activeGroupId,
                 activeTree(layout),
               ),
           }
         );

       let resizeWindowByAxis = (direction, delta) =>
         updateActiveLayout(layout =>
           {
             ...layout,
             tree:
               Layout.resizeWindowByAxis(
                 direction,
                 layout.activeGroupId,
                 delta,
                 activeTree(layout),
               ),
           }
         );

       let resizeWindowByDirection = (direction, delta) =>
         updateActiveLayout(layout =>
           {
             ...layout,
             tree:
               Layout.resizeWindowByDirection(
                 direction,
                 layout.activeGroupId,
                 delta,
                 activeTree(layout),
               ),
           }
         );

       let resetWeights =
         updateActiveLayout(layout =>
           {
             ...layout,
             tree: Layout.resetWeights(activeTree(layout)),
             uncommittedTree: `None,
           }
         );

       let maximize = (~direction=?) =>
         updateActiveLayout(layout =>
           {
             ...layout,
             uncommittedTree:
               `Maximized(
                 Layout.maximize(
                   ~direction?,
                   layout.activeGroupId,
                   activeTree(layout),
                 ),
               ),
           }
         );
     };

let update = (~focus, model, msg) => {
  switch (msg) {
  | SplitDragged({path, delta}) => (
      updateActiveLayout(
        layout => {
          let layout =
            switch (layout.uncommittedTree) {
            | `Maximized(tree) => {...layout, tree}
            | `Resizing(_)
            | `None => layout
            };
          {
            ...layout,
            uncommittedTree:
              `Resizing(Layout.resizeSplit(~path, ~delta, layout.tree)),
          };
        },
        model,
      ),
      Nothing,
    )

  | DragComplete => (updateTree(Fun.id, model), Nothing)

  | GroupTabClicked(id) => (
      updateActiveGroup(Group.select(id), model),
      Nothing,
    )

  | GroupSelected(id) => (
      updateActiveLayout(layout => {...layout, activeGroupId: id}, model),
      Focus(Center),
    )

  | EditorCloseButtonClicked(id) =>
    switch (removeEditor(id, model)) {
    | Some(model) => (model, Nothing)
    | None => (model, RemoveLastBlocked)
    }

  | Command(NextEditor) => (nextEditor(model), Nothing)
  | Command(PreviousEditor) => (previousEditor(model), Nothing)

  | Command(SplitVertical) => (split(`Vertical, model), SplitAdded)

  | Command(SplitHorizontal) => (split(`Horizontal, model), SplitAdded)

  | Command(CloseActiveEditor) =>
    switch (removeActiveEditor(model)) {
    | Some(model) => (model, Nothing)
    | None => (model, RemoveLastBlocked)
    }

  | Command(MoveLeft) =>
    switch (focus) {
    | Some(Center) =>
      let layout = model |> activeLayout;
      let newActiveGroupId =
        layout |> activeTree |> moveLeft(layout.activeGroupId);

      if (newActiveGroupId == layout.activeGroupId) {
        (model, Focus(Left));
      } else {
        (
          updateActiveLayout(
            layout => {...layout, activeGroupId: newActiveGroupId},
            model,
          ),
          Nothing,
        );
      };

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(MoveRight) =>
    switch (focus) {
    | Some(Center) =>
      let model =
        updateActiveLayout(
          layout => {
            let newActiveGroupId =
              layout |> activeTree |> moveRight(layout.activeGroupId);
            {...layout, activeGroupId: newActiveGroupId};
          },
          model,
        );
      (model, Nothing);

    | Some(Left) =>
      let model =
        updateActiveLayout(
          layout => {
            let newActiveGroupId = layout |> activeTree |> Layout.leftmost;
            {...layout, activeGroupId: newActiveGroupId};
          },
          model,
        );
      (model, Focus(Center));

    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(MoveUp) =>
    switch (focus) {
    | Some(Center) =>
      let model =
        updateActiveLayout(
          layout => {
            let newActiveGroupId =
              layout |> activeTree |> moveUp(layout.activeGroupId);
            {...layout, activeGroupId: newActiveGroupId};
          },
          model,
        );
      (model, Nothing);

    | Some(Bottom) =>
      let model =
        updateActiveLayout(
          layout => {
            let newActiveGroupId = layout |> activeTree |> Layout.bottommost;
            {...layout, activeGroupId: newActiveGroupId};
          },
          model,
        );
      (model, Focus(Center));

    | Some(Left)
    | None => (model, Nothing)
    }

  | Command(MoveDown) =>
    switch (focus) {
    | Some(Center) =>
      let layout = model |> activeLayout;
      let newActiveGroupId =
        layout |> activeTree |> moveDown(layout.activeGroupId);

      if (newActiveGroupId == layout.activeGroupId) {
        (model, Focus(Bottom));
      } else {
        (
          updateActiveLayout(
            layout => {...layout, activeGroupId: newActiveGroupId},
            model,
          ),
          Nothing,
        );
      };

    | Some(Left) => (model, Focus(Bottom))

    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(RotateForward) =>
    switch (focus) {
    | Some(Center) => (rotate(`Forward, model), Nothing)

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(RotateBackward) =>
    switch (focus) {
    | Some(Center) => (rotate(`Backward, model), Nothing)

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(DecreaseSize) =>
    switch (focus) {
    | Some(Center) => (
        model
        |> resizeWindowByAxis(`Horizontal, 0.95)
        |> resizeWindowByAxis(`Vertical, 0.95),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(IncreaseSize) =>
    switch (focus) {
    | Some(Center) => (
        model
        |> resizeWindowByAxis(`Horizontal, 1.05)
        |> resizeWindowByAxis(`Vertical, 1.05),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(DecreaseHorizontalSize) =>
    switch (focus) {
    | Some(Center) => (
        model |> resizeWindowByAxis(`Horizontal, 0.95),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(IncreaseHorizontalSize) =>
    switch (focus) {
    | Some(Center) => (
        model |> resizeWindowByAxis(`Horizontal, 1.05),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(DecreaseVerticalSize) =>
    switch (focus) {
    | Some(Center) => (
        model |> resizeWindowByAxis(`Vertical, 0.95),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(IncreaseVerticalSize) =>
    switch (focus) {
    | Some(Center) => (
        model |> resizeWindowByAxis(`Vertical, 1.05),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(IncreaseWindowSize(direction)) =>
    switch (focus) {
    | Some(Center) => (
        model |> resizeWindowByDirection(direction, 1.05),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(DecreaseWindowSize(direction)) =>
    switch (focus) {
    | Some(Center) => (
        model |> resizeWindowByDirection(direction, 0.95),
        Nothing,
      )

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(Maximize) =>
    switch (focus) {
    | Some(Center) => (maximize(model), Nothing)

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(MaximizeHorizontal) =>
    switch (focus) {
    | Some(Center) => (maximize(~direction=`Horizontal, model), Nothing)

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(MaximizeVertical) =>
    switch (focus) {
    | Some(Center) => (maximize(~direction=`Vertical, model), Nothing)

    | Some(Left)
    | Some(Bottom)
    | None => (model, Nothing)
    }

  | Command(ToggleMaximize) =>
    let model =
      switch (activeLayout(model).uncommittedTree) {
      | `Maximized(_) =>
        updateActiveLayout(
          layout => {...layout, uncommittedTree: `None},
          model,
        )

      | _ =>
        switch (focus) {
        | Some(Center) => maximize(model)

        | Some(Left)
        | Some(Bottom)
        | None => model
        }
      };
    (model, Nothing);

  | Command(ResetSizes) => (resetWeights(model), Nothing)

  | Command(AddLayout) => (addLayoutTab(model), Nothing)
  };
};
// VIEW

module type ContentModel = {
  type t = Feature_Editor.Editor.t;

  let id: t => int;
  let title: t => string;
  let icon: t => option(Oni_Core.IconTheme.IconDefinition.t);
  let isModified: t => bool;

  let render: (~isActive: bool, t) => Revery.UI.element;
};

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
  let make =
      (
        ~children as provider,
        ~model,
        ~isZenMode,
        ~showTabs,
        ~uiFont,
        ~theme,
        ~dispatch,
        (),
      ) =>
    component(hooks => {
      let ((maybeDimensions, setDimensions), hooks) =
        Hooks.state(None, hooks);

      let layout = activeLayout(model);
      let tree = activeTree(layout);

      let children =
        switch (maybeDimensions) {
        | Some((width, height)) =>
          let positioned =
            isZenMode
              ? Positioned.fromWindow(
                  0,
                  0,
                  width,
                  height,
                  model.activeGroupId,
                )
              : Positioned.fromLayout(0, 0, width, height, tree);

          let renderWindow = id =>
            switch (groupById(id, layout)) {
            | Some(group) =>
              <EditorGroupView
                provider
                uiFont
                showTabs
                isActive={group.id == layout.activeGroupId}
                theme
                model=group
                dispatch
              />
            | None => React.empty
            };

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

  let nextEditor =
    define(
      ~category="View",
      ~title="Open Next Editor",
      "workbench.action.nextEditor",
      Command(NextEditor),
    );

  let previousEditor =
    define(
      ~category="View",
      ~title="Open Previous Editor",
      "workbench.action.previousEditor",
      Command(PreviousEditor),
    );

  let splitVertical =
    define(
      ~category="View",
      ~title="Split Editor Vertically",
      "view.splitVertical",
      Command(SplitVertical),
    );

  let splitHorizontal =
    define(
      ~category="View",
      ~title="Split Editor Horizontally",
      "view.splitHorizontal",
      Command(SplitHorizontal),
    );

  let closeActiveEditor =
    define(
      ~category="View",
      ~title="Close Editor",
      "view.closeEditor",
      Command(CloseActiveEditor),
    );

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

  let addLayout =
    define(
      ~category="View",
      ~title="Add Layout Tab",
      "oni.layout.add",
      Command(AddLayout),
    );
};

module Contributions = {
  let commands =
    Commands.[
      nextEditor,
      previousEditor,
      splitVertical,
      splitHorizontal,
      closeActiveEditor,
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
      addLayout,
    ];
};
