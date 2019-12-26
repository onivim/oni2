open Revery.UI;
open Revery.UI.Components;

open Oni_Model;

let button = Style.[marginVertical(24)];

let onExplorerClick = _ => {
  GlobalContext.current().dispatch(Actions.ActivityBar(FileExplorerClick));
};

let onSearchClick = _ => {
  GlobalContext.current().dispatch(Actions.ActivityBar(SearchClick));
};

let onExtensionsClick = _ => {
  GlobalContext.current().dispatch(Actions.ActivityBar(ExtensionsClick));
};

let make = (~state: State.t, ()) => {
  let bg = state.theme.activityBarBackground;
  let fg = state.theme.activityBarForeground;

  <View
    style=Style.[
      top(0),
      bottom(0),
      backgroundColor(bg),
      alignItems(`Center),
      width(50),
    ]>
    <Sneakable onClick=onExplorerClick style=button>
      <FontIcon backgroundColor=bg color=fg icon=FontAwesome.file />
    </Sneakable>
    <Sneakable onClick=onSearchClick style=button>
      <FontIcon backgroundColor=bg color=fg icon=FontAwesome.search />
    </Sneakable>
    <Sneakable onClick=onExtensionsClick style=button>
      <FontIcon backgroundColor=bg color=fg icon=FontAwesome.box />
    </Sneakable>
  </View>;
};
