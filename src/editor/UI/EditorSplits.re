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
/**
   TODO:
   1.) convert this to use new direction, simplifies this greatly
 */
let getSplitStyle = split =>
  Style.(
    switch (split) {
    | {direction: Vertical, width: Some(w), _} => [
        top(0),
        bottom(0),
        width(w),
      ]
    | {direction: Vertical, width: None, _} => [
        top(0),
        bottom(0),
        flexGrow(1),
      ]
    | {direction: Horizontal, height: None, _} => [
        left(0),
        right(0),
        flexGrow(1),
      ]
    | {direction: Horizontal, height: Some(h), _} => [
        left(0),
        right(0),
        height(h),
        flexGrow(1),
      ]
    }
  );

let getWidth = width =>
  switch (width) {
  | Some(w) => Style.width(w)
  | None => Style.width(10)
  };

let renderDock = (dock: option(dock), state: State.t) =>
  switch (dock) {
  | Some(d) => [
      <View style=Style.[getWidth(d.width), top(0), bottom(0)]>
        {d.component()}
      </View>,
      <WindowHandle direction=Vertical theme={state.theme} />,
    ]
  | None => [React.empty]
  };

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks => {
    let {editorLayout, theme, _}: State.t = state;
    let splits =
      WindowManager.traverseSplitTree(
        (allSplits, window, direction) => [
          <View style={getSplitStyle(window)}> {window.component()} </View>,
          <WindowHandle direction theme />,
          ...allSplits,
        ],
        [],
        editorLayout.windows,
        Vertical /* Initial split direction, less relevant as we currently start with one split open*/,
      )
      |> List.append(renderDock(editorLayout.leftDock, state));
    (hooks, <View style=splitContainer> ...splits </View>);
  });
