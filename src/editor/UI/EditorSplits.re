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

let getSplitStyle = split =>
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
  | None => []
  };

let parentStyle = (dir: direction) => {
  let flexDir =
    switch (dir) {
    | Vertical => `Row
    | Horizontal => `Column
    };
  Style.[flexGrow(0), flexDirection(flexDir)];
};

let handleParent = (children, direction) => [
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

    let renderSplit = (allSplits, window, direction) => [
      <View style={getSplitStyle(window)}> {window.component()} </View>,
      <WindowHandle direction theme />,
      ...allSplits,
    ];

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
        components =>
          List.concat([
            components,
            renderDock(editorLayout.rightDock, state),
          ])
      );
    (hooks, <View style=splitContainer> ...splits </View>);
  });
