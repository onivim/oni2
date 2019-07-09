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

let rec renderTree = (state, tree) => {
  let items = WindowTreeLayout.layout(0, 0, 400, 400, tree);

  print_endline ("ITEMS: " ++ string_of_int(List.length(items)));

  List.map((item: WindowTreeLayout.t) => {
    <View style=Style.[
     position(`Absolute),
     top(item.y),
     left(item.x),
     width(item.width),
     height(item.height),
    ]>
      <EditorGroupView state editorGroupId={item.split.editorGroupId} />
    </View>
  }, items);
}
let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks => {
    let {State.editorLayout, _} = state;
    let {windows, leftDock, rightDock, _} = editorLayout;

    let children = renderTree(state, windows);

    let splits = renderDock(leftDock, state) 
    @ [<View onDimensionsChanged={(dim) => print_endline("changed: " ++ string_of_int(dim.width))}  style=Style.[flexGrow(1), backgroundColor(Colors.red)]>...children</View>] 
    @ renderDock(rightDock, state);
    
    (hooks, <View style=splitContainer> ...splits </View>);
  });
