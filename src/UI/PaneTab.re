/*
 * PaneTab.re
 *
 * A tab in the lower pane
 */

open Revery.UI;
open Revery.UI.Components;

open Oni_Core;
module Model = Oni_Model;

module Colors = Feature_Theme.Colors;

module Constants = {
  let minWidth = 100;
};

module Styles = {
  open Style;
  open UiFont;

  let container = (~isActive, ~theme) => {
    let modeColor = Colors.Oni.backgroundFor(Mode.Normal).from(theme);
    let borderColor = isActive ? modeColor : Revery.Colors.transparentBlack;

    [
      overflow(`Hidden),
      paddingHorizontal(5),
      backgroundColor(Colors.Editor.background.from(theme)),
      borderTop(~color=Revery.Colors.transparentBlack, ~width=2),
      borderBottom(~color=borderColor, ~width=2),
      height(30),
      minWidth(Constants.minWidth),
      flexDirection(`Row),
      justifyContent(`Center),
      alignItems(`Center),
    ];
  };

  let clickable = [
    flexGrow(1),
    flexDirection(`Row),
    alignItems(`Center),
    justifyContent(`Center),
  ];

  let text = (~isActive, ~uiFont, ~theme) => [
    textOverflow(`Ellipsis),
    fontFamily(isActive ? uiFont.fontFileSemiBold : uiFont.fontFile),
    fontSize(uiFont.fontSize),
    color(Colors.Tab.activeForeground.from(theme)),
    justifyContent(`Center),
    alignItems(`Center),
  ];
};

let make = (~uiFont, ~theme, ~title, ~onClick, ~isActive, ()) => {
  <View style={Styles.container(~isActive, ~theme)}>
    <Clickable onClick style=Styles.clickable>
      <Text style={Styles.text(~isActive, ~uiFont, ~theme)} text=title />
    </Clickable>
  </View>;
};
