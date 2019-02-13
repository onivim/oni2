open Revery.UI;
let component = React.component("overlay");

let createElement = (~children, ()) =>
  component((_slots: React.Hooks.empty) =>
    <View
      style=Style.[
        position(`Absolute),
        top(0),
        right(0),
        left(0),
        bottom(0),
        paddingTop(20),
        alignItems(`Center),
        overflow(LayoutTypes.Hidden),
        flexDirection(`Column),
      ]>
      ...children
    </View>
  );
