/**
   EditorLayoutView

   This modules interprets the current state of the window
   manager and arranges its children accordingly
 */
open Revery;
open UI;
open Oni_Core;
open Oni_Model;
open WindowManager;
open WindowTree;

let splitContainer = Style.[flexGrow(1), flexDirection(`Row)];

let splitStyle = Style.[flexGrow(1)];

let getDockStyle = ({width, _}: dock, theme: Theme.t) => {
  let w =
    switch (width) {
    | Some(w) => w
    | None => 10
    };
  Style.[
    width(w),
    top(0),
    bottom(0),
    backgroundColor(theme.sideBarBackground),
  ];
};

let renderDock = (dockItems: list(dock), state: State.t) =>
  List.sort((prev, curr) => curr.order > prev.order ? 1 : 0, dockItems)
  |> List.fold_left(
       (accum, item) =>
         [
           <View style={getDockStyle(item, state.theme)}>
             {item.component()}
           </View>,
           <WindowHandle direction=Vertical theme={state.theme} />,
           ...accum,
         ],
       [],
     );

let parentStyle = (dir: direction) => {
  let flexDir =
    switch (dir) {
    | Vertical => `Row
    | Horizontal => `Column
    };
  Style.[flexGrow(1), flexDirection(flexDir)];
};

let renderTree = (state, tree) => {
  open State;
  let items =
    WindowTreeLayout.layout(
      0,
      0,
      state.windowManager.windowTreeWidth,
      state.windowManager.windowTreeHeight,
      tree,
    );

  items
  |> List.map((item: WindowTreeLayout.t) =>
       <View
         style=Style.[
           position(`Absolute),
           top(item.y),
           left(item.x),
           width(item.width),
           height(item.height),
         ]>
         <EditorGroupView
           state
           windowId={item.split.id}
           editorGroupId={item.split.editorGroupId}
         />
       </View>
     )
  |> React.listToElement;
};

let make = (~state: State.t, ()) => {
  let {State.windowManager, _} = state;
  let {windowTree, leftDock, rightDock, _} = windowManager;

  let children = renderTree(state, windowTree);

  let splits =
    renderDock(leftDock, state)
    @ [
      <View
        onDimensionsChanged={dim =>
          GlobalContext.current().notifyWindowTreeSizeChanged(
            ~width=dim.width,
            ~height=dim.height,
            (),
          )
        }
        style=Style.[flexGrow(1)]>
        children
      </View>,
    ]
    @ renderDock(rightDock, state)
    |> React.listToElement;

  <View style=splitContainer> splits </View>;
};
