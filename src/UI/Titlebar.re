/*
 * Titlebar.re
 */

open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Core;
module Model = Oni_Model;

module Colors = Feature_Theme.Colors.TitleBar;

module Styles = {
  open Style;

  let container = (~isFocused, ~theme) => [
    flexGrow(0),
    height(25),
    backgroundColor(
      isFocused ?
        Colors.activeBackground.from(theme) :
        Colors.inactiveBackground.from(theme),
    ),
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
  ];

  let text = (~isFocused, ~theme, ~font: UiFont.t) => [
    flexGrow(0),
    fontSize(12.),
    fontFamily(font.fontFileSemiBold),
    backgroundColor(
      isFocused ?
        Colors.activeBackground.from(theme) :
        Colors.inactiveBackground.from(theme),
    ),
    color(
      isFocused ?
        Colors.activeForeground.from(theme) :
        Colors.inactiveForeground.from(theme),
    ),
    textWrap(TextWrapping.NoWrap),
  ];
};

let make = (~isFocused, ~isFullscreen, ~title, ~theme, ~font: UiFont.t, ()) =>
  if (isFullscreen) {
    React.empty;
  } else {
    switch (Revery.Environment.os) {
    | Mac =>
      <Clickable
        onDoubleClick=(
          _ =>
            GlobalContext.current().dispatch(Model.Actions.TitleDoubleClicked)
        )
        style={Styles.container(~isFocused, ~theme)}>
        <Text style={Styles.text(~isFocused, ~theme, ~font)} text=title />
      </Clickable>
    | _ => React.empty
    };
  };
