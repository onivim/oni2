open Revery;
open Revery.UI;

module Styles = {
  open Style;
  let vertical = [
    cursor(MouseCursors.horizontalResize),
    backgroundColor(Color.rgba(0., 0., 0., 0.1)),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
  ];

  let horizontal = [
    cursor(MouseCursors.verticalResize),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
    height(4),
  ];
};

let component = React.Expert.component("handleView");
let handle =
    (~direction, ~onDrag: float => unit, ~onDragComplete: unit => unit, ()) =>
  component(hooks => {
    let ((captureMouse, _state), hooks) =
      Hooks.mouseCapture(
        ~onMouseMove=
          ((originX, originY), evt) => {
            let delta =
              switch (direction) {
              | `Vertical => evt.mouseX -. originX
              | `Horizontal => evt.mouseY -. originY
              };

            onDrag(delta);
            Some((originX, originY));
          },
        ~onMouseUp=
          (_, _) => {
            onDragComplete();
            None;
          },
        (),
        hooks,
      );

    let onMouseDown = (evt: NodeEvents.mouseButtonEventParams) => {
      captureMouse((evt.mouseX, evt.mouseY));
    };

    (
      <View
        onMouseDown
        style={direction == `Vertical ? Styles.vertical : Styles.horizontal}
      />,
      hooks,
    );
  });

module Vertical = {
  let make = handle(~direction=`Vertical);
};

module Horizontal = {
  let make = handle(~direction=`Horizontal);
};
