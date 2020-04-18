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

let parentStyle = (dir: Feature_Layout.WindowTree.direction) => {
  let flexDir =
    switch (dir) {
    | Vertical => `Row
    | Horizontal => `Column
    };
  Style.[flexGrow(1), flexDirection(flexDir)];
};

let renderTree = (state, theme, tree) => {
  open State;
  let items =
    Feature_Layout.WindowTreeLayout.layout(
      0,
      0,
      state.layout.windowTreeWidth,
      state.layout.windowTreeHeight,
      tree,
    );

  items
  |> List.map((item: Feature_Layout.WindowTreeLayout.t) =>
       <View
         style=Style.[
           position(`Absolute),
           top(item.y),
           left(item.x),
           width(item.width),
           height(item.height),
         ]>
         {switch (
            EditorGroups.getEditorGroupById(
              state.editorGroups,
              item.split.editorGroupId,
            )
          ) {
          | Some(editorGroup) => <EditorGroupView state theme editorGroup />
          | None => React.empty
          }}
       </View>
     )
  |> React.listToElement;
};

let make = (~state: State.t, ~theme, ()) => {
  let children = renderTree(state, theme, state.layout.windowTree);

  let splits =
    [
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
    |> React.listToElement;

  <View style=splitContainer> splits </View>;
};
