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

  let countContainer = (~theme) => [
    width(16),
    height(16),
    borderRadius(8.),
    flexGrow(0),
    backgroundColor(Colors.ActivityBarBadge.background.from(theme)),
    paddingRight(4),
    position(`Relative),
  ];

  let countInner = [
    position(`Absolute),
    width(16),
    height(16),
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    top(1),
    left(0),
  ];
};

let numberToString = num => {
  num >= 10 ? "9+" : string_of_int(num);
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

  let fgColor = Colors.SideBarSectionHeader.foreground.from(theme);

  let countForeground = Colors.ActivityBarBadge.foreground.from(theme);

  <View style={Styles.container(expanded)}>
    <Clickable style={Styles.titleBar(theme)} onClick>
      <View style=Styles.chevronContainer>
        <Codicon
          fontSize=Constants.arrowSize
          color=fgColor
          icon={expanded ? Codicon.chevronDown : Codicon.chevronRight}
        />
      </View>
      <View style=Style.[paddingTop(2), flexGrow(1)]>
        <Text
          style={Styles.title(~theme)}
          fontFamily={uiFont.family}
          fontSize=13.
          fontWeight=Revery.Font.Weight.Bold
          text=title
        />
      </View>
      <View style={Styles.countContainer(~theme)}>
        <View style=Styles.countInner>
          <Text
            style=Style.[color(countForeground)]
            fontFamily={uiFont.family}
            fontSize=10.
            fontWeight=Revery.Font.Weight.Bold
            text={numberToString(count)}
          />
        </View>
      </View>
    </Clickable>
    list
  </View>;
};
