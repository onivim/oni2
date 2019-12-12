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

let toggleSearch = ({pane, _}: State.t, _) => {
  let action =
    if (Pane.isTypeOpen(Pane.Search, pane)) {
      Actions.PaneClosed;
    } else {
      Actions.PaneOpen(Pane.Search);
    };
  GlobalContext.current().dispatch(action);
};

let make = (~state: State.t, ()) => {
  let bg = state.theme.activityBarBackground;
  let fg = state.theme.activityBarForeground;

  <View
    style=Style.[
      flexGrow(1),
      top(0),
      bottom(0),
      backgroundColor(bg),
      alignItems(`Center),
    ]>
    <Clickable onClick={toggleExplorer(state)} style=button>
      <FontIcon backgroundColor=bg color=fg icon=FontAwesome.file />
    </Clickable>
    <Clickable onClick={toggleSearch(state)} style=button>
      <FontIcon backgroundColor=bg color=fg icon=FontAwesome.search />
    </Clickable>
  </View>;
};
