/*
 * StatusBar.re
 *
 * Container for StatusBar
 */

open Revery.Core;
open Revery.UI;

open Oni_Core;

let component = React.component("StatusBar");

let textStyle =
  Style.[
    color(Color.hex("#9da5b4")),
    fontFamily("Inter-UI-Regular.ttf"),
    fontSize(14),
  ];

let viewStyle =
  Style.[
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
  ];

let make = (~mode: Types.Mode.t, ()) =>
  component((_slots: React.Hooks.empty) =>
    <View style=viewStyle>
      <Text style=textStyle text={Types.Mode.show(mode)} />
    </View>
  );

let createElement = (~children as _, ~mode, ()) =>
  React.element(make(~mode, ()));
