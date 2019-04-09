open Revery;
open Revery.UI;
open Oni_Core;
open Types.EditorSplits;

let component = React.component("Spacer");

let lastItem = (l, index) => List.length(l) == index + 1;
let spacerColor = Color.rgba(0., 0., 0., 0.1);

let spacer = (layout: layout) => {
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

  switch (layout) {
  | Full => verticalStyles
  | VerticalRight => [marginLeft(1), ...verticalStyles]
  | VerticalLeft => [marginRight(1), ...verticalStyles]
  | HorizontalTop => [marginBottom(1), ...horizontalStyles]
  | HorizontalBottom => [marginTop(1), ...horizontalStyles]
  };
};

let createElement =
    (
      ~children as _,
      ~windowNumber: int,
      ~splits: list(split),
      ~theme as _: Theme.t,
      ~layout: layout,
      (),
    ) =>
  component(hooks =>
    (
      hooks,
      lastItem(splits, windowNumber) ?
        React.empty : <View style={spacer(layout)} />,
    )
  );
