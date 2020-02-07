/*
 * Accordion.re
 */

open Revery;
open Revery.UI;

open Oni_Core;
open Oni_Components;

module Model = Oni_Model;
module Ext = Oni_Extensions;

module Constants = {
  let arrowSize = 15.;
};

module Styles = {
  let container = expanded =>
    Style.[flexGrow(expanded ? 1 : 0), flexDirection(`Column)];

  let titleBar = (theme: Theme.t) =>
    Style.[
      flexGrow(0),
      height(25),
      backgroundColor(theme.editorBackground),
      color(theme.editorForeground),
      flexDirection(`Row),
      alignItems(`Center),
    ];

  let titleText = (~theme: Theme.t, ~font: UiFont.t) =>
    Style.[
      fontSize(font.fontSize),
      fontFamily(font.fontFile),
      color(theme.editorForeground),
      backgroundColor(theme.editorBackground),
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
      ~theme: Theme.t,
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
        color=Colors.white
        icon={expanded ? FontAwesome.caretDown : FontAwesome.caretRight}
        backgroundColor=Colors.transparentWhite
      />
      <Text style={Styles.titleText(~theme, ~font=uiFont)} text=title />
    </View>
    list
  </View>;
};
