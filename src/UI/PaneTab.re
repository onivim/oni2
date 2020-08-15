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

  let container = (~isActive, ~theme) => {
    let borderColor =
      isActive ? Colors.PanelTitle.activeBorder : Colors.Panel.background;

    [
      overflow(`Hidden),
      paddingHorizontal(5),
      backgroundColor(Colors.Panel.background.from(theme)),
      borderBottom(~color=borderColor.from(theme), ~width=2),
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

  let text = (~isActive, ~theme) => [
    textOverflow(`Ellipsis),
    isActive
      ? color(Colors.PanelTitle.activeForeground.from(theme))
      : color(Colors.PanelTitle.inactiveForeground.from(theme)),
    justifyContent(`Center),
    alignItems(`Center),
  ];
};

let make = (~uiFont: UiFont.t, ~theme, ~title, ~onClick, ~isActive, ()) => {
  <View style={Styles.container(~isActive, ~theme)}>
    <Clickable onClick style=Styles.clickable>
      <Text
        style={Styles.text(~isActive, ~theme)}
        fontFamily={uiFont.family}
        fontWeight={isActive ? Medium : Normal}
        fontSize={uiFont.size}
        text=title
      />
    </Clickable>
  </View>;
};
