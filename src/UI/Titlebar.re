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
    fontSize(12),
    fontFamily(font.fontFileSemiBold),
    backgroundColor(background),
    color(foreground),
    textWrap(TextWrapping.NoWrap),
  ];
};

let make = (~title, ~theme: Theme.t, ~font: UiFont.t, ()) => {
  switch (Revery.Environment.os) {
  | Mac =>
    let {
      titleBarActiveBackground,
      titleBarActiveForeground,
      titleBarInactiveBackground,
      titleBarInactiveForeground,
      _,
    }: Theme.t = theme;
    <View style={Styles.container(titleBarActiveBackground)}>
      <Text
        style={Styles.text(
          ~background=titleBarActiveBackground,
          ~foreground=titleBarActiveForeground,
          ~font,
        )}
        text=title
      />
    </View>;
  | _ => React.empty
  };
};
