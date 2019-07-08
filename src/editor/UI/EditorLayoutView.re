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

let component = React.component("EditorSplits");

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
    backgroundColor(theme.colors.sideBarBackground),
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

let rec renderTree = (~direction, theme, state, tree) =>
  switch (tree) {
  | Parent(direction, children) =>
    /* let _c = renderTree(~direction, theme, children, state); */
    /* React.empty */
    <View style={parentStyle(direction)}>
      ...{List.map(renderTree(~direction, theme, state), children)}
    </View>
  | Leaf(window) =>
    <View style=splitStyle>
      <EditorGroupView state editorGroupId={window.editorGroupId} />
      <WindowHandle direction theme />
    </View>
  | Empty => React.empty
  };

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks => {
    let {State.editorLayout, theme, _} = state;
    let {windows, leftDock, rightDock, _} = editorLayout;

    let splits =
      renderDock(rightDock, state)
      |> (@)([renderTree(~direction=Vertical, theme, state, windows)])
      |> (@)(renderDock(leftDock, state));
    (hooks, <View style=splitContainer> ...splits </View>);
  });
