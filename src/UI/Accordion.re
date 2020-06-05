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
  open Style;

  let container = expanded => [
    flexGrow(expanded ? 1 : 0),
    flexDirection(`Column),
  ];

  let titleBar = theme => [
    flexGrow(0),
    height(25),
    backgroundColor(Colors.SideBar.background.from(theme)),
    color(Colors.SideBar.foreground.from(theme)),
    flexDirection(`Row),
    alignItems(`Center),
  ];

  let title = (~theme) => [color(Colors.SideBar.foreground.from(theme))];
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
      ? <FlatList rowHeight count focused theme> ...renderItem </FlatList>
      : React.empty;

  <View style={Styles.container(expanded)}>
    <View style={Styles.titleBar(theme)}>
      <FontIcon
        fontSize=Constants.arrowSize
        color={Colors.foreground.from(theme)}
        icon={expanded ? FontAwesome.caretDown : FontAwesome.caretRight}
      />
      <Text
        style={Styles.title(~theme)}
        fontFamily={uiFont.normal}
        fontSize={uiFont.size}
        text=title
      />
    </View>
    list
  </View>;
};
