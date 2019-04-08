open Revery;
open Revery_UI;

let component = React.component("Dock");

let createElement = (~children as _, ~state as _, ()) =>
  component(hooks =>
    (
      hooks,
      <View
        style=Style.[
          top(0),
          bottom(0),
          width(90),
          backgroundColor(Colors.black),
        ]
      />,
    )
  );
