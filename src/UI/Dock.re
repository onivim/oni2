open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Model;

let component = React.component("Dock");

let button = Style.[marginVertical(24)];

let toggleExplorer = ({fileExplorer, _}: State.t, _) => {
  let action =
    fileExplorer.isOpen
      ? Actions.RemoveDockItem(WindowManager.ExplorerDock)
      : Actions.AddDockItem(WindowManager.ExplorerDock);
  GlobalContext.current().dispatch(action);
};

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks => {
    let bg = state.theme.activityBarBackground;
    let fg = state.theme.activityBarForeground;
    (
      hooks,
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
      </View>,
    );
  });
