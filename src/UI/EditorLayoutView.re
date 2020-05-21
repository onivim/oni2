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

  let verticalHandle = (node: Feature_Layout.sized(_)) => [
    cursor(MouseCursors.horizontalResize),
    position(`Absolute),
    left(node.x + node.width - 5),
    top(node.y),
    width(10),
    height(node.height),
  ];

  let horizontalHandle = (node: Feature_Layout.sized(_)) => [
    cursor(MouseCursors.verticalResize),
    position(`Absolute),
    left(node.x),
    top(node.y + node.height - 5),
    width(node.width),
    height(10),
  ];
};

let%component handleView =
              (~direction, ~node: Feature_Layout.sized(_), ~onDrag, ()) => {
  let%hook (captureMouse, _state) =
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
    );

  let onMouseDown = (evt: NodeEvents.mouseButtonEventParams) => {
    captureMouse((evt.mouseX, evt.mouseY));
  };

  <View
    onMouseDown
    style={
      direction == `Vertical
        ? Styles.verticalHandle(node) : Styles.horizontalHandle(node)
    }
  />;
};

let rec nodeView =
        (
          ~state,
          ~theme,
          ~editorGroups,
          ~path=[],
          ~node: Feature_Layout.sized(_),
          (),
        ) => {
  switch (node.kind) {
  | `Split(direction, children) =>
    let parent = node;

    let rec loop = (index, children) => {
      let path = [index, ...path];

      switch (children) {
      | [] => []
      | [node] => [<nodeView state theme editorGroups path node />]

      | [node, ...[_, ..._] as rest] =>
        let onDrag = delta => {
          let total = direction == `Vertical ? parent.width : parent.height;
          GlobalContext.current().dispatch(
            Actions.WindowHandleDragged({
              path: List.rev(path),
              delta: delta /. float(total) // normalized
            }),
          );
        };
        [
          <nodeView state theme editorGroups path node />,
          <handleView direction node onDrag />,
          ...loop(index + 1, rest),
        ];
      };
    };

    loop(0, children) |> React.listToElement;

  | `Window(id) =>
    switch (EditorGroups.getEditorGroupById(editorGroups, id)) {
    | Some(editorGroup) =>
      <View
        style=Style.[
          position(`Absolute),
          left(node.x),
          top(node.y),
          width(node.width),
          height(node.height),
        ]>
        <EditorGroupView state theme editorGroup />
      </View>
    | None => React.empty
    }
  };
};

let%component make = (~state: State.t, ~theme, ()) => {
  let%hook (maybeDimensions, setDimensions) = Hooks.state(None);
  let children =
    switch (maybeDimensions) {
    | Some((width, height)) =>
      let sizedLayout =
        Feature_Layout.layout(0, 0, width, height, state.layout);

      <nodeView
        state
        theme
        editorGroups={state.editorGroups}
        node=sizedLayout
      />;

    | None => React.empty
    };

  <View
    onDimensionsChanged={dim =>
      setDimensions(_ => Some((dim.width, dim.height)))
    }
    style=Styles.container>
    children
  </View>;
};
