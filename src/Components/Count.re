/*
 * Component_Accordion.re
 */

open Revery.UI;

open Oni_Core;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;
  let container = (~theme) => [
    width(16),
    height(16),
    borderRadius(8.),
    flexGrow(0),
    backgroundColor(Colors.ActivityBarBadge.background.from(theme)),
    position(`Relative),
  ];

  let inner = [
    position(`Absolute),
    width(16),
    height(16),
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    top(1),
    left(0),
  ];
};

let numberToString = num => {
  num >= 10 ? "9+" : string_of_int(num);
};

let make = (~count, ~theme, ~uiFont: UiFont.t, ()) => {
  let countForeground = Colors.ActivityBarBadge.foreground.from(theme);
  <View style={Styles.container(~theme)}>
    <View style=Styles.inner>
      <Text
        style=Style.[color(countForeground)]
        fontFamily={uiFont.family}
        fontSize=10.
        fontWeight=Revery.Font.Weight.Bold
        text={numberToString(count)}
      />
    </View>
  </View>;
};
