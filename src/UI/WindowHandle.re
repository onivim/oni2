open Revery.UI;

let _isLastItem = (splits, index) => List.length(splits) == index + 1;
let spacerColor = Revery.Color.rgba(0., 0., 0., 0.1);

let spacer = (direction: Feature_Layout.WindowTree.direction) => {
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

let make = (~direction: Feature_Layout.WindowTree.direction, ()) =>
  <View style={spacer(direction)} />;
