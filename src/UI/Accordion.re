/*
 * Accordion.re
 */

open Revery.UI;

open Oni_Core;
open Oni_Components;

module Model = Oni_Model;
module Ext = Oni_Extensions;

module Colors = Feature_Theme.Colors;

module Constants = {
  let arrowSize = 15.;
};

module Styles = {
  let container = expanded =>
    Style.[flexGrow(expanded ? 1 : 0), flexDirection(`Column)];

  let titleBar = theme =>
    Style.[
      flexGrow(0),
      height(25),
      backgroundColor(Colors.SideBar.background.from(theme)),
      color(Colors.SideBar.foreground.from(theme)),
      flexDirection(`Row),
      alignItems(`Center),
    ];

  let titleText = (~theme, ~font: UiFont.t) =>
    Style.[
      fontSize(font.fontSize),
      fontFamily(font.fontFile),
      color(Colors.SideBar.foreground.from(theme)),
    ];
};

let make =
    (
      ~title,
      ~rowHeight,
      ~expanded,
      ~count,
      ~renderItem,
      ~focused,
      ~theme,
      ~uiFont: UiFont.t,
      (),
    ) => {
  let list =
    expanded
      ? <FlatList rowHeight count focused> ...renderItem </FlatList>
      : React.empty;

  <View style={Styles.container(expanded)}>
    <View style={Styles.titleBar(theme)}>
      <FontIcon
        fontSize=Constants.arrowSize
        color=Revery.Colors.white
        icon={expanded ? FontAwesome.caretDown : FontAwesome.caretRight}
      />
      <Text style={Styles.titleText(~theme, ~font=uiFont)} text=title />
    </View>
    list
  </View>;
};
