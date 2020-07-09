/*
 * Accordion.re
 */

open Revery.UI;
open Revery.UI.Components;

open Oni_Core;

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

  let chevronContainer = [padding(4), flexGrow(0)];

  let titleBar = theme => [
    flexGrow(0),
    height(25),
    backgroundColor(Colors.SideBarSectionHeader.background.from(theme)),
    flexDirection(`Row),
    alignItems(`Center),
  ];

  let title = (~theme) => [
    color(Colors.SideBarSectionHeader.foreground.from(theme)),
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
      ~onClick,
      (),
    ) => {
  let list =
    expanded
      ? <FlatList rowHeight count focused theme> ...renderItem </FlatList>
      : React.empty;

  <View style={Styles.container(expanded)}>
    <Clickable style={Styles.titleBar(theme)} onClick>
      <View style=Styles.chevronContainer>
        <Codicon
          fontSize=Constants.arrowSize
          color={Colors.SideBarSectionHeader.foreground.from(theme)}
          icon={expanded ? Codicon.chevronDown : Codicon.chevronRight}
        />
      </View>
      <View style=Style.[paddingTop(2)]>
        <Text
          style={Styles.title(~theme)}
          fontFamily={uiFont.family}
          fontSize=13.
          fontWeight=Revery.Font.Weight.Bold
          text=title
        />
      </View>
    </Clickable>
    list
  </View>;
};
