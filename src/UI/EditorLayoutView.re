/**
   EditorLayoutView

   This modules interprets the current state of the window
   manager and arranges its children accordingly
 */
open Revery;
open UI;
open Oni_Model;

let splitContainer = Style.[flexGrow(1), flexDirection(`Row)];

let splitStyle = Style.[flexGrow(1)];

let parentStyle = direction => {
  let flexDir =
    switch (direction) {
    | `Vertical => `Row
    | `Horizontal => `Column
    };
  Style.[flexGrow(1), flexDirection(flexDir)];
};

let renderTree = (~width, ~height, state: State.t, theme, tree) => {
  let items = Feature_Layout.layout(0, 0, width, height, tree);

  items
  |> List.map((item: Feature_Layout.sizedWindow(_)) =>
       <View
         style=Style.[
           position(`Absolute),
           top(item.y),
           left(item.x),
           width(item.width),
           height(item.height),
         ]>
         {switch (
            EditorGroups.getEditorGroupById(state.editorGroups, item.content)
          ) {
          | Some(editorGroup) => <EditorGroupView state theme editorGroup />
          | None => React.empty
          }}
       </View>
     )
  |> React.listToElement;
};

let%component make = (~state: State.t, ~theme, ()) => {
  let%hook (maybeDimensions, setDimensions) = Hooks.state(None);
  let children =
    switch (maybeDimensions) {
    | Some((width, height)) =>
      renderTree(~width, ~height, state, theme, state.layout)
    | None => React.empty
    };

  let splits =
    [
      <View
        onDimensionsChanged={dim =>
          setDimensions(_ => Some((dim.width, dim.height)))
        }
        style=Style.[flexGrow(1)]>
        children
      </View>,
    ]
    |> React.listToElement;

  <View style=splitContainer> splits </View>;
};
