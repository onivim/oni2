open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Model;

let button = Style.[marginVertical(24)];

let toggleExplorer = ({fileExplorer, _}: State.t, _) => {
  let action =
    fileExplorer.isOpen
      ? Actions.RemoveDockItem(WindowManager.ExplorerDock)
      : Actions.AddDockItem(WindowManager.ExplorerDock);
  GlobalContext.current().dispatch(action);
};

let make = (~state: State.t, ()) => {
  let bg = state.theme.editorLineNumberBackground;

  <View
    style=Style.[
      flexGrow(1),
      top(0),
      bottom(0),
      backgroundColor(bg),
      alignItems(`Center),
    ]>
    <Clickable onClick={toggleExplorer(state)} style=button>
      <FontIcon backgroundColor=bg color=Colors.white icon=FontAwesome.file />
    </Clickable>
  </View>;
};
