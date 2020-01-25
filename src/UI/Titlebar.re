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
    height(22),
    backgroundColor(background),
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
  ];

  let text = (~background, ~foreground, ~font: UiFont.t) => [
    flexGrow(0),
    fontSize(font.fontSize),
    fontFamily(font.fontFile),
    backgroundColor(background),
    color(foreground),
    textWrap(TextWrapping.NoWrap),
  ];
};

let make = (~title, ~theme: Theme.t, ~font: UiFont.t, ()) => {
  switch (Revery.Environment.os) {
  | Mac => let {background, foreground, _}: Theme.t = theme;
  <View style={Styles.container(background)}>
    <Text style={Styles.text(~background, ~foreground, ~font)} text=title />
  </View>;
  | _ => React.empty
  }
};
