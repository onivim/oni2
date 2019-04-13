/**
   Editor Splits

   This modules interprets the current state of the window
   manager and arranges its children accordingly
 */
open Revery;
open UI;
open Oni_Model;
open WindowManager;

let component = React.component("EditorSplits");

let splitContainer = Style.[flexGrow(1), flexDirection(`Row)];

let getSplitStyle = (split: split) =>
  Style.(
    switch (split) {
    | {direction: Vertical, width: Some(w), _} => [width(w)]
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
    (accum, item) => [
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

let handleParent = (direction, children) => [
  <View style={parentStyle(direction)}> ...children </View>,
];

let renderSplit = (~theme, allSplits, window, direction) => [
  <View style={getSplitStyle(window)}> {window.component()} </View>,
  <WindowHandle direction theme />,
  ...allSplits,
];

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks => {
    let {State.editorLayout, theme, _} = state;

    let splits =
      WindowManager.traverseSplitTree(
        ~result=[],
        ~handleParent,
        ~action=renderSplit(~theme),
        ~tree=editorLayout.windows,
        ~direction=Vertical /* Initial split direction, less relevant as we currently start with one split open*/,
        (),
      )
      |> List.append(renderDock(editorLayout.leftDock, state))
      |> (
        components => components @ renderDock(editorLayout.rightDock, state)
      );
    (hooks, <View style=splitContainer> ...splits </View>);
  });
