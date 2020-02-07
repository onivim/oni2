/*
 * Titlebar.re
 */

open Revery;
open Revery.UI;

open Oni_Core;
module Model = Oni_Model;

module Styles = {
  open Style;

  let container = background => [
    flexGrow(0),
    height(25),
    backgroundColor(background),
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
  ];

  let text = (~background, ~foreground, ~font: UiFont.t) => [
    flexGrow(0),
    fontSize(12.),
    fontFamily(font.fontFileSemiBold),
    backgroundColor(background),
    color(foreground),
    textWrap(TextWrapping.NoWrap),
  ];
};

let make =
    (~focused, ~maximized, ~title, ~theme: Theme.t, ~font: UiFont.t, ()) =>
  if (maximized) {
    React.empty;
  } else {
    switch (Revery.Environment.os) {
    | Mac =>
      let {
        titleBarActiveBackground,
        titleBarActiveForeground,
        titleBarInactiveBackground,
        titleBarInactiveForeground,
        _,
      }: Theme.t = theme;
      let background =
        focused ? titleBarActiveBackground : titleBarInactiveBackground;
      let foreground =
        focused ? titleBarActiveForeground : titleBarInactiveForeground;

      <View style={Styles.container(background)}>
        <Text
          style={Styles.text(~background, ~foreground, ~font)}
          text=title
        />
      </View>;
    | _ => React.empty
    };
  };
