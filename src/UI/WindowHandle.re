open Revery.UI;
open Oni_Model;
open WindowTree;

module Core = Oni_Core;

let component = React.component("Spacer");

let _isLastItem = (splits, index) => List.length(splits) == index + 1;
let spacerColor = Revery.Color.rgba(0., 0., 0., 0.1);

let spacer = (direction: direction) => {
  open Style;
  let verticalStyles = [
    backgroundColor(spacerColor),
    width(1),
    top(0),
    bottom(0),
    flexGrow(0),
  ];

  let horizontalStyles = [
    backgroundColor(spacerColor),
    height(1),
    left(0),
    right(0),
    flexGrow(0),
  ];

  switch (direction) {
  | Vertical => [marginHorizontal(1), ...verticalStyles]
  | Horizontal => [marginVertical(1), ...horizontalStyles]
  };
};

let createElement =
    (~children as _, ~theme as _: Core.Theme.t, ~direction: direction, ()) =>
  component(hooks => (hooks, <View style={spacer(direction)} />));
