/**
   EditorLayoutView

   This modules interprets the current state of the window
   manager and arranges its children accordingly
 */
open Revery;
open UI;
open Oni_Model;
open WindowManager;

let component = React.component("EditorSplits");

let splitContainer = Style.[flexGrow(1), flexDirection(`Row)];

let splitStyle = (split: split) =>
  Style.(
    switch (split) {
    | {direction: Vertical, width: Some(w), _} => [width(w), flexGrow(1)]
    | {direction: Vertical, width: None, _} => [flexGrow(1)]
    | {direction: Horizontal, height: None, _} => [flexGrow(1)]
    | {direction: Horizontal, height: Some(h), _} => [
        height(h),
        flexGrow(1),
      ]
    }
  );

let getDockStyle = ({width, _}: dock) => {
  let w =
    switch (width) {
    | Some(w) => w
    | None => 10
    };
  Style.[width(w), top(0), bottom(0)];
};

let renderDock = (dockItems: list(dock), state: State.t) =>
  List.fold_left(
    (accum, item) =>
      [
        <View style={getDockStyle(item)}> {item.component()} </View>,
        <WindowHandle direction=Vertical theme={state.theme} />,
        ...accum,
      ],
    [],
    dockItems,
  );

let parentStyle = (dir: direction) => {
  let flexDir =
    switch (dir) {
    | Vertical => `Row
    | Horizontal => `Column
    };
  Style.[flexGrow(1), flexDirection(flexDir)];
};

let rec renderTree = (~direction, theme, tree, state) =>
  switch (tree) {
  | Parent(direction, _, children) =>
    <View style={parentStyle(direction)}>
      renderTree(~direction, theme), children)
    </View>
  | Leaf(window) =>
    <View style={splitStyle(window)}>
      <EditorGroupView state editorGroupId=window.editorGroupId />
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
      |> (@)([renderTree(~direction=Vertical, theme, windows, state)])
      |> (@)(renderDock(leftDock, state));
    (hooks, <View style=splitContainer> ...splits </View>);
  });
