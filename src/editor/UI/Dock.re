open Revery;
open Revery_UI;
open Revery.UI.Components;

open Oni_Model;

let component = React.component("Dock");

let button = Style.[marginVertical(8)];

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks => {
    let bg = state.theme.colors.editorLineNumberBackground;
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
        <Clickable style=button>
          <FontIcon
            backgroundColor=bg
            color=Colors.white
            icon=FontAwesome.file
          />
        </Clickable>
        <Clickable style=button>
          <FontIcon
            backgroundColor=bg
            color=Colors.white
            icon=FontAwesome.search
          />
        </Clickable>
      </View>,
    );
  });
