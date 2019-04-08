open Revery_UI;
open Revery.UI.Components;
open Oni_Model;

let component = React.component("Dock");

let dockItemSize = 20;

let iconStyles =
  Style.[fontFamily("FontAwesome5FreeSolid.otf"), fontSize(dockItemSize)];

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
          width(90),
          backgroundColor(bg),
          alignItems(`Center),
        ]>
        <Clickable style=button>
          <Text text={||} style=iconStyles />
        </Clickable>
        <Clickable style=button>
          <Text text={||} style=iconStyles />
        </Clickable>
      </View>,
    );
  });
