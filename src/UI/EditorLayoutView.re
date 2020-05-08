/**
   EditorLayoutView

   This modules interprets the current state of the window
   manager and arranges its children accordingly
 */
open Revery;
open UI;
open Oni_Model;

module Styles = {
  open Style;

  let container = [flexGrow(1), flexDirection(`Row)];

  let split = direction => [
    flexDirection(direction == `Vertical ? `Row : `Column),
    backgroundColor(direction == `Vertical ? Colors.red : Colors.blue),
    flexGrow(1),
  ];

  let handle = direction => [
    direction == `Vertical ? width(3) : height(3),
    cursor(
      direction == `Vertical
        ? MouseCursors.horizontalResize : MouseCursors.verticalResize,
    ),
  ];
};

let rec nodeView =
        (~state, ~theme, ~editorGroups, ~node: Feature_Layout.t(_), ()) => {
  switch (node) {
  | Split(direction, _, children) =>
    <View style={Styles.split(direction)}>
      {children
       |> List.map(node => <nodeView state theme editorGroups node />)
       |> Base.List.intersperse(
            ~sep=<View style={Styles.handle(direction)} />,
          )
       |> React.listToElement}
    </View>

  | Window(_, id) =>
    switch (EditorGroups.getEditorGroupById(editorGroups, id)) {
    | Some(editorGroup) =>
//      <EditorGroupView state theme editorGroup />
      <View style=Style.[backgroundColor(Colors.green |> Color.multiplyAlpha(0.5)), flexGrow(1)] />
    | None => React.empty
    }
  };
};

let make = (~state: State.t, ~theme, ()) => {
  <View style=Styles.container>
    <nodeView
      state
      theme
      editorGroups={state.editorGroups}
      node={state.layout}
    />
  </View>;
};
