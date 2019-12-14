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
         {switch (
            EditorGroups.getEditorGroupById(
              state.editorGroups,
              item.split.editorGroupId,
            )
          ) {
          | Some(editorGroup) =>
            <EditorGroupView state windowId={item.split.id} editorGroup />
          | None => React.empty
          }}
       </View>
     )
  |> React.listToElement;
};

let make = (~state: State.t, ()) => {
  let {State.windowManager, _} = state;
  let {windowTree, _} = windowManager;

  let children = renderTree(state, windowTree);

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
